import matplotlib.pyplot as plt
import csv

def plot_from_csv(filename):
    with open(filename, 'r') as file:
        reader = csv.reader(file)
        data = [list(map(float, row)) for row in reader]

    x_values = data[0]
    y_values1 = data[1]
    y_values2 = data[2]

    plt.figure(figsize=(10, 5))
    plt.plot(x_values, y_values1, marker='*', color='purple', linestyle='-', label="real execution time")
    plt.plot(x_values, y_values2, marker='*', color='blue', linestyle='-', label="estimated excution time")

    plt.xlabel("CPUs")
    plt.ylabel("Time (seconds)")
    plt.title("Line Plot from CSV Data")
    
    plt.yticks([i * 0.000005 for i in range(1, 6)])

    plt.legend()
    plt.grid(True)
    # plt.show()
    plt.savefig('output_plot.png')

plot_from_csv('output.csv')
