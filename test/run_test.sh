#!/bin/bash

line_no=1
while IFS= read -r line; do
    echo "Line $line_no: $line"
    echo ""
    echo "$line" | ./test.out
    echo ""
    line_no=$((line_no + 1))
done < test_cases.txt