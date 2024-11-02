#!/bin/bash

if ! (make clean && make && make run); then
    exit 1
fi

input_file="time.log"
output_file="output.csv"

if [[ ! -f $input_file ]]; then
    echo "Input file $input_file does not exist."
    exit 1
fi

{
    first_row=$(awk '{print NR}' "$input_file" | paste -sd, -)
    echo "$first_row"

    time_values=$(awk '{print $0}' "$input_file" | paste -sd, -)
    echo "$time_values"

    first_time_value=$(echo "$time_values" | cut -d',' -f1)

    new_third_row=""
    IFS=',' read -r -a first_row_array <<< "$first_row"
    
    for j in "${!first_row_array[@]}"; do
        current_index_value="${first_row_array[j]}"
        if [[ "$current_index_value" != "0" ]]; then
            new_third_value=$(echo "scale=6; $first_time_value / $current_index_value" | bc -l)
            new_third_row+=$(printf "%.6f" "$new_third_value"),
        else
            new_third_row+="Inf,"  
        fi
    done
    new_third_row=${new_third_row%,}
    echo "$new_third_row"

    division_row=""
    IFS=',' read -r -a time_values_array <<< "$time_values"
    for time_value in "${time_values_array[@]}"; do
        if [[ "$time_value" != "0" ]]; then
            division_result=$(echo "scale=6; $time_value / 2" | bc -l)
            division_row+=$(printf "%.6f" "$division_result"),
        else
            division_row+="Inf,"
        fi
    done
    division_row=${division_row%,}
    echo "$division_row"

} > "$output_file"

