# coding=UTF8

from __future__ import print_function
import sys
from environment.Breakout import Breakout
from rl_brain import DeepQNetwork
import cv2
import numpy as np
import datetime

MAX_EPISODE = 100000
N_ACTIONS = 4
MEMORY_SIZE = 50000
MINIBATCH_SIZE = 32
GAMMA = 0.99
INITIAL_EPSILON = 0

def preprocess(observation):
    "将游戏画面转换成黑白并且调整图片大小"
    observation = cv2.cvtColor(cv2.resize(observation, (84, 84)), cv2.COLOR_BGR2GRAY)
    ret, observation = cv2.threshold(observation, 1, 255, cv2.THRESH_BINARY)
    return observation

def main():
    begin_time = datetime.datetime.now()

    env = Breakout()
    # env = env.unwrapped
    brain = DeepQNetwork(
        n_actions=N_ACTIONS,
        memory_size=MEMORY_SIZE,
        minibatch_size=MINIBATCH_SIZE,
        gamma=GAMMA,
        epsilon=INITIAL_EPSILON
    )

    step = 0
    for episode in range(MAX_EPISODE):
        # do nothing
        observation = env.reset()
        # observation, _, _, _ = env.step(1)
        observation = preprocess(observation)
        brain.reset(observation)
        total_reward = 0
        while True:
            # env.render()
            action = brain.choose_action()
            # print(action, type(action))
            next_observation, reward, done, _ = env.step(action)
            if reward != 0:
                total_reward += reward
                # print("reward: {} action: {}".format(reward, action))
            # print("reward:", reward)
            next_observation = preprocess(next_observation)
            brain.store_transition(observation, action, reward, done, next_observation)
            # 有一定的记忆就可以开始学习了
            if step > 200:
                brain.learn()

            if done:
                break

            observation = next_observation
            step += 1

        end_time = datetime.datetime.now()
        print("episode {} over. exec time:{} step:{} total_reward:{}".format(episode, end_time - begin_time, step, total_reward))

    env.exit("game over")


if __name__ == "__main__":
    if len(sys.argv) == 2 and sys.argv[1] == 'train':
        INITIAL_EPSILON = 0.1
    main()
