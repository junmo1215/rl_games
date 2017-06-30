# coding:UTF-8

"""
网络架构
"""

from __future__ import print_function
import random
import numpy as np
import tensorflow as tf


class DeepQNetwork:
    def __init__(
            self,
            n_actions,
            memory_size,
            minibatch_size,
            gamma,
            epsilon
    ):
        self.n_actions = n_actions
        self.memory_size = memory_size
        self.memory_list = []
        self.minibatch_size = minibatch_size
        self.time_step = 0
        self.gamma = gamma
        self.epsilon = epsilon

        self.current_state = None

        self._create_network()

    def reset(self, observation):
        self.current_state = np.stack((observation, observation, observation, observation), axis=2)

    def store_transition(self, observation, action, reward, done, observation_):
        "储存要训练的内容"
        next_state = np.append(observation_, self.current_state[:, :, :3], axis=2)

        self.memory_list.append([self.current_state, action, reward, done, next_state])
        if len(self.memory_list) > self.memory_size:
            del self.memory_list[0]
        self.time_step += 1
        self.current_state = next_state

    def choose_action(self, observation):
        "根据当前状态选择要执行的动作"
        n_actions = self.n_actions

        action = np.zeros(n_actions)
        if random.random() <= self.epsilon:
            action_index = random.randrange(n_actions)
            action[action_index] = 1
        else:
            # 只输入了一个状态
            q_value = self.q_value.eval(feed_dict={self.state_input: [self.current_state]})[0]
            action_index = np.argmax(q_value)
            action[action_index] = 1

        # 减去一个很小的数，让训练快结束的时候随机的次数越来越少
        # 这里写了两个奇怪的数字，要考虑能不能写成time_step的函数
        if self.epsilon > 0.0001:
            self.epsilon -= 3.33e-8

        return action

    def learn(self):
        "训练步骤"
        minibatch = random.sample(self.memory_list, self.minibatch_size)

        state_batch = [data[0] for data in minibatch]
        action_batch = [data[1] for data in minibatch]
        reward_batch = [data[2] for data in minibatch]
        # 第三列记录的内容是是否终止
        next_state_batch = [data[4] for data in minibatch]

        act_q_value_batch = []
        next_q_value_batch = self.q_value.eval(feed_dict={self.state_input: next_state_batch})
        for i in range(self.minibatch_size):
            act_q_value = reward_batch[2]
            # 下一个状态没有终止的话还需要加上之后状态的衰减
            if not minibatch[i][3]:
                act_q_value += self.gamma * np.max(next_q_value_batch[i])
            act_q_value_batch.append(act_q_value)

        self.train_step.run(feed_dict={
            self.action_input: action_batch,
            self.state_input: state_batch,
            self.act_q_value: act_q_value_batch
        })

        # save network every 10000 iteration
        if self.time_step % 10000 == 0:
            self.saver.save(self.session, 'saved_networks/network-dqn', global_step=self.time_step)

    def _create_network(self):
        # network weights
        w_conv1 = self._weight_variable([8, 8, 4, 32])
        b_conv1 = self._bias_variable([32])

        w_conv2 = self._weight_variable([4, 4, 32, 64])
        b_conv2 = self._bias_variable([64])

        w_conv3 = self._weight_variable([3, 3, 64, 64])
        b_conv3 = self._bias_variable([64])

        w_fc1 = self._weight_variable([1600, 512])
        b_fc1 = self._bias_variable([512])

        w_fc2 = self._weight_variable([512, self.n_actions])
        b_fc2 = self._bias_variable([self.n_actions])

        # input layer
        # 当前状态
        self.state_input = tf.placeholder("float", [None, 80, 80, 4])

        # hidden layers
        h_conv1 = tf.nn.relu(self._conv2d(self.state_input, w_conv1, 4) + b_conv1)
        h_pool1 = self._max_pool_2x2(h_conv1)

        h_conv2 = tf.nn.relu(self._conv2d(h_pool1, w_conv2, 2) + b_conv2)

        h_conv3 = tf.nn.relu(self._conv2d(h_conv2, w_conv3, 1) + b_conv3)

        h_conv3_flat = tf.reshape(h_conv3, [-1, 1600])
        h_fc1 = tf.nn.relu(tf.matmul(h_conv3_flat, w_fc1) + b_fc1)

        # Q Value layer
        self.q_value = tf.matmul(h_fc1, w_fc2) + b_fc2

        # 当前状态选择的action
        self.action_input = tf.placeholder("float", [None, self.n_actions])
        # 当前状态实际的q_value
        self.act_q_value = tf.placeholder("float", [None])
        # 按照经验预测出的q_value(Q(s_t, a_t))
        pred_q_value = tf.reduce_sum(tf.multiply(self.q_value, self.action_input),
                                     axis=1)
        self.cost = tf.reduce_mean(tf.square(self.act_q_value - pred_q_value))
        self.train_step = tf.train.AdamOptimizer(1e-6).minimize(self.cost)

        # saving and loading networks
        self.saver = tf.train.Saver()
        self.session = tf.InteractiveSession()
        self.session.run(tf.global_variables_initializer())

        checkpoint = tf.train.get_checkpoint_state("saved_networks")
        if checkpoint and checkpoint.model_checkpoint_path:
            self.saver.restore(self.session, checkpoint.model_checkpoint_path)
            self.time_step = self._get_last_time_step(checkpoint.model_checkpoint_path)
            print("Successfully loaded:", checkpoint.model_checkpoint_path)
        else:
            print("Could not find old network weights")

    def _get_last_time_step(self, str_model_checkpoint_path):
        return int(str_model_checkpoint_path.split('-')[-1])

    def _weight_variable(self, shape):
        initial = tf.truncated_normal(shape, stddev=0.01)
        return tf.Variable(initial)

    def _bias_variable(self, shape):
        initial = tf.constant(0.01, shape=shape)
        return tf.Variable(initial)

    def _conv2d(self, x, w, stride):
        return tf.nn.conv2d(x, w, strides=[1, stride, stride, 1], padding="SAME")

    def _max_pool_2x2(self,x):
        return tf.nn.max_pool(x, ksize=[1, 2, 2, 1], strides=[1, 2, 2, 1], padding="SAME")
