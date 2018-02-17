# coding = UTF8

import matplotlib.pyplot as plt
import numpy as np

BEGIN_PATH = 0
ENG_PATH = 2800000

def draw_score():
    xs = []
    avg_scores = []
    min_scores = []
    max_scores = []
    for i in np.arange(BEGIN_PATH, ENG_PATH, 50000):
        x = i + 50000
        data = np.load(r"evaluate_results/{}.npy".format(x)).item()
        x = x / 50000
        xs.append(x)
        min_scores.append(data["min_score"])
        max_scores.append(data["max_score"])
        avg_scores.append(data["avg_score"])
    lines = plt.plot(
        xs, min_scores,
        xs, max_scores,
        xs, avg_scores)

    plt.setp(lines[0], linewidth=1)
    plt.setp(lines[1], linewidth=1)
    plt.setp(lines[2], linewidth=1)

    plt.legend(('min score', 'max score', 'avg score'),
           loc='upper right')
    plt.title('Average scores on Breakout')
    plt.show()

def draw_avg_q():
    xs = []
    avg_qs = []
    for i in np.arange(BEGIN_PATH, ENG_PATH, 50000):
        x = i + 50000
        data = np.load(r"evaluate_results/{}.npy".format(x)).item()
        x = x / 50000
        xs.append(x)
        avg_qs.append(data["avg_q"])
    lines = plt.plot(
        xs, avg_qs)

    plt.setp(lines[0], linewidth=1)

    # plt.legend(('avg Q'),
    #        loc='upper right')
    plt.title('Average Q on Breakout')
    plt.show()

if __name__ == "__main__":
    draw_score()
    draw_avg_q()
