# coding:UTF-8

from __future__ import print_function
import game.wrapped_flappy_bird as game
from rl_brain import DeepQNetwork
import cv2
import numpy as np
import datetime

MAX_EPISODE = 100000
N_ACTIONS = 2
MEMORY_SIZE = 50000
MINIBATCH_SIZE = 32
GAMMA = 0.99
INITIAL_EPSILON = 0.1

def preprocess(observation, reshape):
    "将游戏画面转换成黑白并且调整图片大小"
    observation = cv2.cvtColor(cv2.resize(observation, (80, 80)), cv2.COLOR_BGR2GRAY)
    ret, observation = cv2.threshold(observation, 1, 255, cv2.THRESH_BINARY)
    if reshape:
        return np.reshape(observation, (80, 80, 1))
    else:
        return observation

def main():
    begin_time = datetime.datetime.now()

    env = game.GameState()
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
        observation, _, _ = env.frame_step([1, 0])
        observation = preprocess(observation, False)
        brain.reset(observation)
        while True:
            action = brain.choose_action(observation)
            observation_, reward, done = env.frame_step(action)
            observation_ = preprocess(observation_, True)
            brain.store_transition(observation, action, reward, done, observation_)
            # 有一定的记忆就可以开始学习了
            if step > 200:
                brain.learn()

            if done:
                break

            observation = observation_
            step += 1

        end_time = datetime.datetime.now()
        print("episode {} over. exec time:{} step:{}".format(episode, end_time - begin_time, step))

    env.exit("game over")


if __name__ == "__main__":
    main()
