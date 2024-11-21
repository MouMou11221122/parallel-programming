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

    new_fourth_row=""
    IFS=',' read -r -a time_values_array <<< "$time_values"
    for time_value in "${time_values_array[@]}"; do
        if [[ "$time_value" != "0" ]]; then
            division_result=$(echo "scale=6; $first_time_value / $time_value" | bc -l)
            new_fourth_row+=$(printf "%.6f" "$division_result"),
        else
            new_fourth_row+="Inf,"
        fi
    done
    new_fourth_row=${new_fourth_row%,}
    echo "$new_fourth_row"

    new_fifth_row=""
    IFS=',' read -r -a fourth_row_array <<< "$new_fourth_row"
    for i in "${!fourth_row_array[@]}"; do
        fourth_value="${fourth_row_array[i]}"
        first_row_value="${first_row_array[i]}"

        if [[ "$fourth_value" != "0" && "$first_row_value" != "0" ]]; then
            part1=$(echo "scale=6; 1 / $fourth_value" | bc -l)
            part2=$(echo "scale=6; 1 / $first_row_value" | bc -l)
            numerator=$(echo "scale=6; $part1 - $part2" | bc -l)
            denominator=$(echo "scale=6; 1 - $part2" | bc -l)

            fifth_value=$(echo "scale=6; $numerator / $denominator" | bc -l)
            new_fifth_row+=$(printf "%.6f" "$fifth_value"),
        else
            new_fifth_row+="Inf,"
        fi
    done
    new_fifth_row=${new_fifth_row%,}
    echo "$new_fifth_row"

} > "$output_file"

