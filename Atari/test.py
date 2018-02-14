# coding=UTF8

from __future__ import print_function
import sys
from environment.Breakout import Breakout
from rl_brain import DeepQNetwork
import cv2
import numpy as np
import datetime

MAX_EPISODE = 100
N_ACTIONS = 4
INITIAL_EPSILON = 0

def preprocess(observation):
    "将游戏画面转换成黑白并且调整图片大小"
    observation = cv2.cvtColor(cv2.resize(observation, (84, 84)), cv2.COLOR_BGR2GRAY)
    ret, observation = cv2.threshold(observation, 1, 255, cv2.THRESH_BINARY)
    return observation

def main(model_path):
    begin_time = datetime.datetime.now()

    env = Breakout()
    # env = env.unwrapped
    # model_path = "saved_networks/network-dqn-350000"
    brain = DeepQNetwork(
        n_actions=N_ACTIONS,
        model_path=model_path,
        init_exploration=INITIAL_EPSILON,
        is_training=False
    )

    max_score = 0
    min_score = 999999999
    total_score = 0
    for episode in range(MAX_EPISODE):
        # do nothing
        observation = env.reset()
        observation = preprocess(observation)
        brain.reset(observation)
        score = 0
        while True:
            env.render()
            action = brain.choose_action()

            next_observation, reward, done, _ = env.step(action)
            if reward >= 0:
                score += reward

            next_observation = preprocess(next_observation)
            brain.store_transition(observation, action, reward, done, next_observation)

            if done:
                break

            observation = next_observation

        total_score += score
        if min_score > score:
            min_score = score
        if max_score < score:
            max_score = score
        print("episode {} over. score:{}".format(episode, score))

    end_time = datetime.datetime.now()
    print("model path: {}. exec time:{}. score range: {} ~ {}. avg: {}".format(model_path, end_time - begin_time, min_score, max_score, total_score / MAX_EPISODE))


if __name__ == "__main__":
    model_path = "saved_networks/network-dqn-1650000"
    main(model_path)
