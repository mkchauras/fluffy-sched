#!/bin/bash

# Proper Scheduler Benchmark Using stress-ng Metrics
# Compares throughput, operations/sec, and other performance metrics

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  Scheduler Metrics Benchmark${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

if [ "$EUID" -ne 0 ]; then 
    echo -e "${RED}Error: Must run as root${NC}"
    exit 1
fi

if [ ! -f "./simple_scheduler" ]; then
    echo -e "${RED}Error: simple_scheduler not found${NC}"
    exit 1
fi

if ! command -v stress-ng &> /dev/null; then
    echo -e "${YELLOW}Installing stress-ng...${NC}"
    if command -v dnf &> /dev/null; then
        dnf install -y stress-ng
    elif command -v apt-get &> /dev/null; then
        apt-get install -y stress-ng
    fi
fi

NCPU=$(nproc)
echo "System: $(uname -r)"
echo "CPUs: $NCPU"
echo ""

RESULTS="metrics_comparison.txt"
echo "Scheduler Metrics Comparison - $(date)" > "$RESULTS"
echo "System: $(uname -r), CPUs: $NCPU" >> "$RESULTS"
echo "========================================" >> "$RESULTS"
echo "" >> "$RESULTS"

# Function to extract metrics from stress-ng output
extract_metrics() {
    local output_file=$1
    local test_name=$2
    local scheduler=$3
    
    echo "" >> "$RESULTS"
    echo "=== $test_name [$scheduler] ===" >> "$RESULTS"
    
    # Extract all metrics
    cat "$output_file" >> "$RESULTS"
    
    # Display to console - show the actual data lines
    echo -e "${GREEN}Metrics for $test_name [$scheduler]:${NC}"
    grep -A 1 "bogo ops/s" "$output_file" | tail -1 || echo "No metrics found"
    echo ""
}

run_test() {
    local test_name=$1
    local stress_cmd=$2
    local scheduler=$3
    local output_file="/tmp/${test_name}_${scheduler}.txt"
    
    echo -e "${YELLOW}Running: $test_name with $scheduler${NC}"
    
    # Run stress-ng with metrics - capture both stdout and stderr
    eval "$stress_cmd --metrics-brief --times" &> "$output_file"
    
    # Extract and display metrics
    extract_metrics "$output_file" "$test_name" "$scheduler"
}

stabilize() {
    sleep 3
}

echo -e "${BLUE}=== Test 1: CPU Stress (4 workers) ===${NC}"
echo "Measures: CPU operations per second"
echo ""

run_test "CPU-4workers" "stress-ng --cpu 4 --timeout 15s" "EEVDF"
stabilize

./simple_scheduler > /tmp/sched.log 2>&1 &
SPID=$!
sleep 2

run_test "CPU-4workers" "stress-ng --cpu 4 --timeout 15s" "Simple"

kill -INT $SPID 2>/dev/null || true
wait $SPID 2>/dev/null || true
stabilize

echo -e "${BLUE}=== Test 2: CPU Stress (20 workers - balanced) ===${NC}"
echo "Measures: Performance at full CPU utilization"
echo ""

run_test "CPU-20workers" "stress-ng --cpu 20 --timeout 15s" "EEVDF"
stabilize

./simple_scheduler > /tmp/sched.log 2>&1 &
SPID=$!
sleep 2

run_test "CPU-20workers" "stress-ng --cpu 20 --timeout 15s" "Simple"

kill -INT $SPID 2>/dev/null || true
wait $SPID 2>/dev/null || true
stabilize

echo -e "${BLUE}=== Test 3: CPU Stress (40 workers - oversubscribed) ===${NC}"
echo "Measures: Scheduler efficiency under contention"
echo ""

run_test "CPU-40workers" "stress-ng --cpu 40 --timeout 15s" "EEVDF"
stabilize

./simple_scheduler > /tmp/sched.log 2>&1 &
SPID=$!
sleep 2

run_test "CPU-40workers" "stress-ng --cpu 40 --timeout 15s" "Simple"

kill -INT $SPID 2>/dev/null || true
wait $SPID 2>/dev/null || true
stabilize

echo -e "${BLUE}=== Test 4: Context Switch Performance ===${NC}"
echo "Measures: Context switches per second"
echo ""

run_test "ContextSwitch" "stress-ng --switch 8 --timeout 15s" "EEVDF"
stabilize

./simple_scheduler > /tmp/sched.log 2>&1 &
SPID=$!
sleep 2

run_test "ContextSwitch" "stress-ng --switch 8 --timeout 15s" "Simple"

kill -INT $SPID 2>/dev/null || true
wait $SPID 2>/dev/null || true
stabilize

echo -e "${BLUE}=== Test 5: Fork Performance ===${NC}"
echo "Measures: Process creation throughput"
echo ""

run_test "Fork" "stress-ng --fork 8 --timeout 15s" "EEVDF"
stabilize

./simple_scheduler > /tmp/sched.log 2>&1 &
SPID=$!
sleep 2

run_test "Fork" "stress-ng --fork 8 --timeout 15s" "Simple"

kill -INT $SPID 2>/dev/null || true
wait $SPID 2>/dev/null || true
stabilize

echo -e "${BLUE}=== Test 6: Matrix Operations ===${NC}"
echo "Measures: Computational throughput"
echo ""

run_test "Matrix" "stress-ng --matrix 4 --timeout 15s" "EEVDF"
stabilize

./simple_scheduler > /tmp/sched.log 2>&1 &
SPID=$!
sleep 2

run_test "Matrix" "stress-ng --matrix 4 --timeout 15s" "Simple"

kill -INT $SPID 2>/dev/null || true
wait $SPID 2>/dev/null || true
stabilize

echo -e "${BLUE}=== Test 7: Scheduling Latency ===${NC}"
echo "Measures: Scheduler responsiveness"
echo ""

run_test "Yield" "stress-ng --yield 8 --timeout 15s" "EEVDF"
stabilize

./simple_scheduler > /tmp/sched.log 2>&1 &
SPID=$!
sleep 2

run_test "Yield" "stress-ng --yield 8 --timeout 15s" "Simple"

kill -INT $SPID 2>/dev/null || true
wait $SPID 2>/dev/null || true

echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${GREEN}Benchmark Complete!${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Create summary comparison
echo "" >> "$RESULTS"
echo "========================================" >> "$RESULTS"
echo "SUMMARY COMPARISON" >> "$RESULTS"
echo "========================================" >> "$RESULTS"
echo "" >> "$RESULTS"

# Extract bogo ops/s for comparison
echo "Operations per Second Comparison:" >> "$RESULTS"
echo "" >> "$RESULTS"

for test in CPU-4workers CPU-20workers CPU-40workers ContextSwitch Fork Matrix Yield; do
    echo "--- $test ---" >> "$RESULTS"
    grep "bogo ops/s" /tmp/${test}_EEVDF.txt 2>/dev/null | head -1 | sed 's/^/  EEVDF:  /' >> "$RESULTS"
    grep "bogo ops/s" /tmp/${test}_Simple.txt 2>/dev/null | head -1 | sed 's/^/  Simple: /' >> "$RESULTS"
    echo "" >> "$RESULTS"
done

echo -e "${GREEN}Full results saved to: $RESULTS${NC}"
echo ""
echo -e "${YELLOW}Key Metrics to Compare:${NC}"
echo "1. bogo ops/s (operations per second) - Higher is better"
echo "2. Context switches - Shows scheduling overhead"
echo "3. CPU migrations - Shows load balancing"
echo "4. usr/sys time ratio - Shows kernel overhead"
echo ""
echo "View full results: cat $RESULTS"

# Made with Bob
