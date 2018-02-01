# coding=UTF8

import os
import gym
import cv2
import numpy as np
from .helper import subimage_in_box

i = 0
def show(image):
    cv2.imshow('img', image)
    cv2.waitKey(0)
    cv2.destroyAllWindows()

class Breakout:
    MAIN_CONTENT_BOX = (6, 30, 147, 204)
    REMAINING_LIFE_BOX = (99, 4, 113, 16)
    MAX_LIFE = 5

    def __init__(self, config=None):
        self.env = gym.make("Breakout-v0")
        # self.remaining_life = Breakout.MAX_LIFE
        # remaining_life_img_format = os.path.join("environment", "assets", "images", "Breakout_remaining_life_{}.png")
        # self.remaining_life_imgs = []
        # for i in range(5):
        #     self.remaining_life_imgs.append(
        #         cv2.imread(remaining_life_img_format.format(i + 1))
        #     )
        self.remaining_life_img = None

    def reset(self):
        self.env.reset()
        # 不知道为什么刚开始训练的时候会一直不发球，然后就卡在这个位置不动
        observation, _, _, _ = self.env.step(1)
        self.remaining_life_img = subimage_in_box(observation, Breakout.REMAINING_LIFE_BOX)
        return observation

    def render(self):
        self.env.render()

    def step(self, action):
        next_observation, reward, done, info = self.env.step(action)
        next_observation, diff_reward, parsed_done = self.__parse_observation(next_observation)
        reward = reward + diff_reward
        done = done or parsed_done
        return next_observation, reward, done, info

    def __parse_observation(self, observation):
        # 掉命的状态算作是终止状态
        remaining_life_img = subimage_in_box(observation, Breakout.REMAINING_LIFE_BOX)
        done = False
        reward = 0
        if np.array_equal(remaining_life_img, self.remaining_life_img) == False:
            # self.remaining_life -= 1
            reward = -1
            done = True
        # global i
        # if (i > 0) and (i % 10 == 0):
        #     observation = cv2.cvtColor(observation, cv2.COLOR_BGR2GRAY)
        #     cv2.imwrite(os.path.join("temp", "test_{}.png".format(i)), observation)
        # i += 1
        # if i > 900:
        #     raise
        return subimage_in_box(observation, Breakout.MAIN_CONTENT_BOX), reward, done
