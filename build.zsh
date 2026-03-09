#!/usr/bin/env zsh
# Build script for Suno Visualizer
# Supports low-spec hardware optimization

emulate -L zsh
setopt errexit nounset pipefail

# Color definitions
typeset -r RED=$'\033[0;31m'
typeset -r GREEN=$'\033[0;32m'
typeset -r YELLOW=$'\033[0;33m'
typeset -r BLUE=$'\033[0;34m'
typeset -r MAGENTA=$'\033[0;35m'
typeset -r CYAN=$'\033[0;36m'
typeset -r WHITE=$'\033[0;37m'
typeset -r BOLD=$'\033[1m'
typeset -r RESET=$'\033[0m'

# Build configuration
typeset -i JOBS=0
typeset BUILD_TYPE="Release"
typeset CLEAN_BUILD=false
typeset VERBOSE=false
typeset LOW_SPEC=false

print_banner() {
    print "${CYAN}${BOLD}"
    print "╔═══════════════════════════════════════════════════════╗"
    print "║       Suno Visualizer Build System                    ║"
    print "║       Professional A/V DAW Context Manager            ║"
    print "╚═══════════════════════════════════════════════════════╝"
    print "${RESET}"
}

detect_hardware() {
    if [[ -r /proc/cpuinfo ]]; then
        local cpu_count=$(grep -c '^processor' /proc/cpuinfo 2>/dev/null || print "1")
        local mem_total=$(awk '/MemTotal/ {print int($2/1024)}' /proc/meminfo 2>/dev/null || print "0")
        
        print "${BLUE}Hardware Detection:${RESET}"
        print "  CPUs: ${WHITE}${cpu_count}${RESET}"
        print "  RAM: ${WHITE}${mem_total}MB${RESET}"
        
        if (( cpu_count <= 2 || mem_total < 4096 )); then
            LOW_SPEC=true
            print "${YELLOW}Low-spec hardware detected${RESET}"
        fi
    fi
}

get_optimal_jobs() {
    if (( JOBS > 0 )); then
        print $JOBS
        return
    fi
    
    local cpu_count=2
    if [[ -r /proc/cpuinfo ]]; then
        cpu_count=$(grep -c '^processor' /proc/cpuinfo 2>/dev/null || print "1")
    fi
    
    if ${LOW_SPEC}; then
        print $(( cpu_count < 2 ? 1 : cpu_count - 1 ))
    else
        print $(( cpu_count + 1 ))
    fi
}

parse_args() {
    zparseopts -D -E -- \
        h=help_opt -help=help_opt \
        j:=jobs_opt \
        d=debug_opt -debug=debug_opt \
        c=clean_opt -clean=clean_opt \
        v=verbose_opt -verbose=verbose_opt \
        l=lowspec_opt -lowspec=lowspec_opt
    
    if (( ${#help_opt} )); then
        print "${BOLD}Usage:${RESET} ${0:t} [OPTIONS]"
        print ""
        print "${BOLD}Options:${RESET}"
        print "  -h, --help      Show this help message"
        print "  -j, --jobs N    Number of parallel jobs (default: auto-detect)"
        print "  -d, --debug     Build in Debug mode (default: Release)"
        print "  -c, --clean     Perform a clean build"
        print "  -v, --verbose   Ver cmake output"
        print "  -l, --lowspec   Optimize for low-spec hardware"
        print ""
        print "${BOLD}Examples:${RESET}"
        print "  ${GREEN}${0:t}${RESET}                    # Standard build"
        print "  ${GREEN}${0:t} -l${RESET}                 # Low-spec optimized"
        print "  ${GREEN}${0:t} -c -j2${RESET}             # Clean build with 2 jobs"
        print "  ${GREEN}${0:t} -d -v${RESET}              # Debug with verbose output"
        exit 0
    fi
    
    if (( ${#jobs_opt} )); then
        JOBS=${jobs_opt[2]}
    fi
    
    if (( ${#debug_opt} )); then
        BUILD_TYPE="Debug"
    fi
    
    if (( ${#clean_opt} )); then
        CLEAN_BUILD=true
    fi
    
    if (( ${#verbose_opt} )); then
        VERBOSE=true
    fi
    
    if (( ${#lowspec_opt} )); then
        LOW_SPEC=true
    fi
}

wait_for_key() {
    print "${CYAN}Press any key to continue...${RESET}"
    read -k 1 -s
}

confirm_action() {
    local prompt="${1:?}"
    print "${YELLOW}${prompt}${RESET} [y/N] "
    read -k 1 -s
    case $REPLY in
        y|Y) print "${GREEN}y${RESET}"; return 0 ;;
        *)   print "${RED}n${RESET}"; return 1 ;;
    esac
}

configure_cmake() {
	local build_dir
	build_dir="${(L)BUILD_TYPE}"
	local jobs
	jobs=$(get_optimal_jobs)
    
    print "${BLUE}Configuring build...${RESET}"
    print "  Build Type: ${WHITE}${BUILD_TYPE}${RESET}"
    print "  Jobs: ${WHITE}${jobs}${RESET}"
    print "  Low-spec: ${WHITE}${LOW_SPEC}${RESET}"
    
    local cmake_args=(
        -B "build/${build_dir}"
        -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    )
    
    if ${LOW_SPEC}; then
        cmake_args+=(
            -DCMAKE_CXX_FLAGS="-O2 -march=native -mtune=generic"
        )
    fi
    
    if ${VERBOSE}; then
        cmake_args+=(--verbose)
    fi
    
    cmake "${cmake_args[@]}"
}

build_project() {
	local build_dir
	build_dir="${(L)BUILD_TYPE}"
	local jobs
	jobs=$(get_optimal_jobs)
    
    print "${BLUE}Building project...${RESET}"
    print "  Using ${WHITE}${jobs}${RESET} parallel jobs"
    
    local build_args=(
        --build "build/${build_dir}"
        --parallel ${jobs}
    )
    
    if ${VERBOSE}; then
        build_args+=(--verbose)
    fi
    
    cmake "${build_args[@]}"
    
    print "${GREEN}${BOLD}✓ Build complete!${RESET}"
    print "  Binary: ${WHITE}build/${build_dir}/suno-visualizer${RESET}"
}

run_clean() {
    print "${YELLOW}Cleaning build directories...${RESET}"
    if [[ -d "build" ]]; then
        rm -rf build
        print "${GREEN}✓ Clean complete${RESET}"
    else
        print "${BLUE}No build directory found${RESET}"
    fi
}

main() {
    print_banner
    detect_hardware
    parse_args "$@"
    
    if ${CLEAN_BUILD}; then
        run_clean
    fi
    
    configure_cmake
    build_project
}

main "$@"
