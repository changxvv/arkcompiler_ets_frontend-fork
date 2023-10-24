#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2023 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import stat
import time
import zipfile

import numpy as np
import pandas as pd

import performance_build
import performance_config


class MailHelper():
    def __init__(self):
        self.pic_table = {}
        self.head_line = []
        self.mail_msg = ''
        self.logs_file = {}
        self.failed_prjs = ''
        MailHelper.remove_files()

    @staticmethod
    def remove_files():
        if os.path.exists(performance_config.MailPicConfig.html_file_path):
            os.remove(performance_config.MailPicConfig.html_file_path)
        if os.path.exists(performance_config.MailPicConfig.attach_path):
            os.remove(performance_config.MailPicConfig.attach_path)
        if not os.path.exists(performance_config.MailPicConfig.mail_data_path):
            os.mkdir(performance_config.MailPicConfig.mail_data_path)
        for build_mode in range(2):
                for log_type in range(3):
                    pic_path = MailHelper.find_in_double_map(build_mode,
                                                             log_type,
                                                             performance_config.MailPicConfig.mail_pic_name,
                                                             "mail_pic_name")
                    if not pic_path:
                        return
                    if os.path.exists(pic_path):
                        os.remove(pic_path)

    def create(self, prj_name):
        self.create_table(prj_name)

    def create_table(self, prj_name):
        self.pic_table[prj_name] = {
            # full_time, incremental_time, size
            performance_config.BuildMode.DEBUG:[np.nan, np.nan, np.nan],
            performance_config.BuildMode.RELEASE:[np.nan, np.nan, np.nan]
        }
        self.head_line.append(prj_name)
        self.time_index = [time.strftime("%Y/%m/%d", time.localtime())]

    @staticmethod
    def find_in_double_map(key_1, key_2, double_dic, error_table_name):
        it_1 = double_dic.get(key_1)
        if it_1:
            it_2 = it_1.get(key_2)
            if not it_2:
                print(f"Can not find key_2: {key_2} in {error_table_name}, please check")
            return it_2
        else:
            print(f"Can not find key_1: {key_1} in {error_table_name}, please check")
            return it_1

    def add_msg(self, msg):
        self.mail_msg += msg

    def add_failed_project(self, prj_name, build_mode, log_type):
        build_mode_info = 'Debug' if build_mode == performance_config.BuildMode.DEBUG else 'Release'
        build_type_info = 'full_build' if log_type == performance_config.LogType.FULL else 'incremental_build'
        info = '-'.join((prj_name, build_mode_info, build_type_info)) + '<br>'
        if self.failed_prjs == '':
            self.failed_prjs = 'failed projects:<br>'
        self.failed_prjs += info

    def add_logs_file(self, filename, buffer):
        self.logs_file[filename] = buffer

    def add_pic_data(self, prj_name, is_debug, data_list):
        build_mode = performance_config.BuildMode.DEBUG if is_debug else performance_config.BuildMode.RELEASE
        prj_table = self.find_in_double_map(prj_name, build_mode, self.pic_table, "pic_table")
        if prj_table:
            if len(data_list) == 1: # size
                prj_table[2] = data_list[0]
            else: # time
                prj_table[0] = data_list[0]
                prj_table[1] = data_list[1]

    def create_msg_file(self):
        with os.fdopen(
            os.open(performance_config.MailPicConfig.html_file_path,
                    os.O_WRONLY | os.O_CREAT | os.O_TRUNC,
                    stat.S_IRWXU | stat.S_IRWXO | stat.S_IRWXG),
                    'w'
            ) as msg_file:
            if self.failed_prjs == '':
                self.failed_prjs = 'All Build Succeed<br>'
            self.mail_msg = performance_config.get_html_prefix().format(self.failed_prjs) + \
                            self.mail_msg + \
                            performance_config.get_html_suffix()
            msg_file.write(self.mail_msg)

    def create_logs_file(self):
        mail_zip = zipfile.ZipFile(performance_config.MailPicConfig.attach_path, 'w')
        for file in self.logs_file:
            mail_zip.writestr(file, self.logs_file.get(file))
        mail_zip.close()

    @staticmethod
    def draw_pic(df, pic_name, title_name, y_lable):
        ax = df.plot(
                    linestyle='-',
                    linewidth=2,
                    marker='o',
                    markersize=6,
                    markeredgecolor='black',
                    title=title_name,
                    ylabel=y_lable,
                    grid=True
        )
        i = 0
        for idx, data in df.iterrows():
            for k in data:
                if not pd.isna(k):
                    ax.text(i + 0.05, k + 0.1, k)
            i += 1
        ax.get_figure().savefig(pic_name)
        
    def create_pic(self):
        for build_mode in range(2):
            for log_type in range(3):
                title_name = MailHelper.find_in_double_map(build_mode,
                                                           log_type,
                                                           performance_config.MailPicConfig.mail_pic_table_lable,
                                                           "mail_pic_table_lable")
                if not title_name:
                    continue
                pic_name = MailHelper.find_in_double_map(build_mode,
                                                         log_type,
                                                         performance_config.MailPicConfig.mail_pic_name,
                                                         "mail_pic_name")
                if not pic_name:
                    continue
                df = None
                csv_filename = MailHelper.find_in_double_map(build_mode,
                                                             log_type,
                                                             performance_config.MailPicConfig.mail_pic_table_name,
                                                             "pic_table_name")
                if not csv_filename:
                    continue
                if os.path.exists(csv_filename):
                    df = pd.read_csv(csv_filename, index_col=0)
                else:
                    df = pd.DataFrame(columns=self.head_line)
                dic = {}
                for prj_name in self.pic_table:
                    dic[prj_name] = [self.pic_table[prj_name][build_mode][log_type]]
                df_inserted = pd.DataFrame(dic, index=self.time_index)
                df = df._append(df_inserted)
                if len(df) > 5:
                    df = df[1:len(df)]
                df.to_csv(csv_filename)
                y_lable = 'build time (s)' if log_type < performance_config.LogType.SIZE else 'size (Byte)'
                self.draw_pic(df, pic_name, title_name, y_lable)

    def create_mail_files(self):
        self.create_msg_file()
        self.create_pic()
        self.create_logs_file()


class PerformanceEntry():
    def __init__(self) -> None:
        self.mail_helper = MailHelper()

    def run(self):
        self.init()
        # You can control which project you need to test here, the first param is the key in performance_config.py
        projects = performance_config.Config.run_list
        for prj in projects:
            self.start_test(prj)
        os.chdir(os.path.dirname(__file__))
        self.create_mail_files()

    def start_test(self, index):
        config = performance_config.get_config(index)
        if not config:
            return
        self.mail_helper.create(os.path.basename(config.project_path))
        performance_build.run(config, self.mail_helper)

    def create_mail_files(self):
        if performance_config.Config.send_mail:
            self.mail_helper.create_mail_files()

    def init(self):
        self.mail_helper = MailHelper()


if __name__ == '__main__':
    strat_time = time.time()
    PerformanceEntry().run()
    print("All test finished at %s\ntotal cost: %ds"
          % (time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), time.time() - strat_time))