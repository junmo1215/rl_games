# coding=UTF8

import os
import gym
import cv2

from .helper import subimage_in_box

i = 0

class Breakout:
    MAIN_CONTENT_BOX = (6, 30, 147, 204)
    REMAINING_LIFE_BOX = ()

    def __init__(self, config=None):
        self.env = gym.make("Breakout-v0")
        self.remaining_life = 5

    def reset(self):
        return self.env.reset()

    def render(self):
        self.env.render()

    def step(self, action):
        next_observation, reward, done, info = self.env.step(action)
        self.__parse_observation(next_observation)
        return next_observation, reward, done, info

    def __parse_observation(self, observation):
        
        global i
        if (i > 0) and (i % 10 == 0):
            observation = cv2.cvtColor(observation, cv2.COLOR_BGR2GRAY)
            cv2.imwrite(os.path.join("temp", "test_{}.png".format(i)), observation)
        i += 1
        if i > 900:
            raise
        return subimage_in_box(observation, Breakout.MAIN_CONTENT_BOX)
