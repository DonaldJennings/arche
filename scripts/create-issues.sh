#!/usr/bin/env bash
# =============================================================================
# create-issues.sh
#
# Generic script that creates GitHub labels, milestones, and issues from a
# JSON data file. No issue content is hardcoded here — edit the data file to
# add, remove, or modify issues.
#
# Usage:
#   ./scripts/create-issues.sh                                  # use default data file
#   ./scripts/create-issues.sh --data-file=scripts/data/my.json # use a custom data file
#   ./scripts/create-issues.sh --dry-run                        # preview without changes
#   ./scripts/create-issues.sh --help
#
# Data file format: scripts/data/issues.json
#
# Requirements:
#   gh  — GitHub CLI  (winget install GitHub.cli && gh auth login)
#   jq  — JSON tool   (winget install stedolan.jq)
# =============================================================================

set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DATA_FILE="${SCRIPT_DIR}/data/issues.json"
DRY_RUN=false

# --- Argument parsing --------------------------------------------------------

for arg in "$@"; do
    case $arg in
        --dry-run)         DRY_RUN=true ;;
        --data-file=*)     DATA_FILE="${arg#*=}" ;;
        --help)
            sed -n '2,14p' "$0" | sed 's/^# \{0,1\}//'
            exit 0
            ;;
        *) echo "Unknown argument: $arg  (use --help for usage)"; exit 1 ;;
    esac
done

# --- Colours -----------------------------------------------------------------

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
CYAN='\033[0;36m'
NC='\033[0m'

log_info()  { echo -e "${GREEN}[INFO]${NC}  $1"; }
log_warn()  { echo -e "${YELLOW}[WARN]${NC}  $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }
log_dry()   { echo -e "${CYAN}[DRY]${NC}   $1"; }

# --- Prerequisites -----------------------------------------------------------

check_prereqs() {
    local ok=true

    if ! command -v gh &> /dev/null; then
        log_error "gh CLI not found. Install: winget install GitHub.cli  then: gh auth login"
        ok=false
    elif ! gh auth status &> /dev/null; then
        log_error "gh CLI not authenticated. Run: gh auth login"
        ok=false
    fi

    if ! command -v jq &> /dev/null; then
        log_error "jq not found. Install: winget install stedolan.jq"
        ok=false
    fi

    if [ ! -f "$DATA_FILE" ]; then
        log_error "Data file not found: ${DATA_FILE}"
        ok=false
    fi

    $ok || exit 1
}

# --- Label helpers -----------------------------------------------------------

create_label() {
    local name="$1" color="$2" description="$3"

    if $DRY_RUN; then
        log_dry "Label: ${name}  (#${color})"
        return
    fi

    if gh label list --repo "$REPO" --limit 200 2>/dev/null | grep -q "^${name}	"; then
        log_warn "Label '${name}' already exists, skipping."
    else
        gh label create "$name" \
            --color "$color" \
            --description "$description" \
            --repo "$REPO" 2>/dev/null \
            && log_info "Created label: ${name}" \
            || log_warn "Could not create label '${name}' (may already exist)."
    fi
}

# --- Milestone helpers -------------------------------------------------------

create_milestone() {
    local title="$1" description="$2"

    if $DRY_RUN; then
        log_dry "Milestone: ${title}"
        return
    fi

    local existing
    existing=$(gh api "repos/${REPO}/milestones" \
        --jq ".[] | select(.title == \"${title}\") | .number" 2>/dev/null || true)

    if [ -n "$existing" ]; then
        log_warn "Milestone '${title}' already exists (#${existing}), skipping."
    else
        gh api "repos/${REPO}/milestones" \
            --method POST \
            --field title="$title" \
            --field description="$description" > /dev/null \
            && log_info "Created milestone: ${title}" \
            || log_warn "Could not create milestone '${title}'."
    fi
}

# --- Issue helpers -----------------------------------------------------------

create_issue() {
    local title="$1" milestone="$2" labels="$3" body="$4"

    if $DRY_RUN; then
        log_dry "Issue: ${title}  [${milestone}] (${labels})"
        return
    fi

    echo "$body" | gh issue create \
        --title "$title" \
        --body-file - \
        --milestone "$milestone" \
        --label "$labels" \
        --repo "$REPO" > /dev/null \
        && log_info "Created issue: ${title}" \
        || log_warn "Could not create issue '${title}'."
}

# --- Main --------------------------------------------------------------------

main() {
    check_prereqs

    REPO=$(jq -r '.repo' "$DATA_FILE")

    echo ""
    echo "  Arche — GitHub issue setup"
    echo "  Repo:      ${REPO}"
    echo "  Data file: ${DATA_FILE}"
    $DRY_RUN && echo "  Mode:      DRY RUN (no changes will be made)"
    echo ""

    # Labels
    local label_count
    label_count=$(jq '.labels | length' "$DATA_FILE")
    log_info "Creating ${label_count} labels..."
    jq -c '.labels[]' "$DATA_FILE" | while IFS= read -r item; do
        create_label \
            "$(echo "$item" | jq -r '.name')" \
            "$(echo "$item" | jq -r '.color')" \
            "$(echo "$item" | jq -r '.description')"
    done

    echo ""

    # Milestones
    local milestone_count
    milestone_count=$(jq '.milestones | length' "$DATA_FILE")
    log_info "Creating ${milestone_count} milestones..."
    jq -c '.milestones[]' "$DATA_FILE" | while IFS= read -r item; do
        create_milestone \
            "$(echo "$item" | jq -r '.title')" \
            "$(echo "$item" | jq -r '.description')"
    done

    echo ""

    # Issues
    local issue_count
    issue_count=$(jq '.issues | length' "$DATA_FILE")
    log_info "Creating ${issue_count} issues..."
    jq -c '.issues[]' "$DATA_FILE" | while IFS= read -r item; do
        create_issue \
            "$(echo "$item" | jq -r '.title')" \
            "$(echo "$item" | jq -r '.milestone')" \
            "$(echo "$item" | jq -r '.labels | join(",")')" \
            "$(echo "$item" | jq -r '.body')"
    done

    echo ""
    log_info "Done. View issues at: https://github.com/${REPO}/issues"
}

main
