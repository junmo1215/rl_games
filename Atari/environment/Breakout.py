# coding=UTF8

import os
import gym
import cv2
import numpy as np
import time
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
    # 一局游戏允许的时间，防止开局一直没有发球
    TIME_LIMIT = 300

    def __init__(self, config=None):
        # v0版的有时候会重复之前的动作，可能发球的这个动作直接被忽略了
        # Gym repeats the current action between 2-4 times, with
        # the number selected randomly.
        # https://github.com/openai/gym/issues/559
        # https://github.com/openai/gym/blob/master/gym/envs/__init__.py#L330
        self.env = gym.make("Breakout-v4")
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
        self.begin_time = time.time()
        self.remaining_life_img = subimage_in_box(observation, Breakout.REMAINING_LIFE_BOX)
        return observation

    def render(self):
        self.env.render()

    def step(self, action):
        next_observation, reward, done, info = self.env.step(action)
        next_observation, diff_reward, parsed_done = self.__parse_observation(next_observation)
        reward = reward + diff_reward
        done = done or parsed_done

        # 检查单局时间是否超出限制，如果超出了直接算结束，并且给出-1的reward
        if time.time() - self.begin_time > Breakout.TIME_LIMIT:
            done = True
            reward = -1

        return next_observation, reward, done, info

    def __parse_observation(self, observation):
        # 掉命的状态给一个-1的reward，但是游戏不结束
        remaining_life_img = subimage_in_box(observation, Breakout.REMAINING_LIFE_BOX)
        done = False
        reward = 0
        if np.array_equal(remaining_life_img, self.remaining_life_img) == False:
            observation, _, _, _ = self.env.step(1)
            # self.remaining_life -= 1
            self.remaining_life_img = remaining_life_img
            reward = -1
            done = False
        # global i
        # if (i > 0) and (i % 10 == 0):
        #     observation = cv2.cvtColor(observation, cv2.COLOR_BGR2GRAY)
        #     cv2.imwrite(os.path.join("temp", "test_{}.png".format(i)), observation)
        # i += 1
        # if i > 900:
        #     raise
        return subimage_in_box(observation, Breakout.MAIN_CONTENT_BOX), reward, done

    def exit(self, meg):
        return self.env.exit(msg)
