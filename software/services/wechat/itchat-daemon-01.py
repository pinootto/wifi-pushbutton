#!/usr/bin/python
# coding=utf-8


# Copyright (C) 2017 by Giovanni Di Mingo <pino_otto@yahoo.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.


import itchat
import time
import json
import web
import subprocess

# users' nicknames
testUserNickName = 'Mamma'
yinzNickName = u'\u4f73\u6850'
waterSupplierNickName = 'Mamma'  #TODO change it to the real one

# groups' nicknames

englishGroup01NickName = 'Upper Intermediate'
englishGroup02NickName = 'Pre-Intermediate'

# define the RESTful endpoints
# /water	-> send message to water bottle supplier
# /english	-> send english fortune message to english group
# /test		-> send test message

urls = (
    '/(.*)', 'ActionController'
)

# define the web service
app = web.application(urls, globals())


class ActionController:
    def GET(self, action):
        status = ""
        if action == 'water':
            print "water"
            msg = u'请送一瓶到我家'
            # print msg
            # itchat.send_msg(msg, 'filehelper')
            send_msg_to_person(msg, waterSupplierNickName)
        elif action == 'english':
            print "english"
            msg = get_english_fortune()
            print msg
            send_msg_to_group(msg, englishGroup01NickName)
            send_msg_to_group(msg, englishGroup02NickName)
        elif action == 'test':
            print "test"
            msg = get_english_fortune()
            print msg
            send_msg_to_person(msg, testUserNickName)
        elif action == 'filehelper':
            print "filehelper"
            #msg = get_english_fortune()
            msg = u'请送一瓶到我家'
            #print msg
            itchat.send_msg(msg, 'filehelper')
        else:
            print "unknown command"
        return json.dumps(status)


def login_callback():
    print('Login successful')


def logout_callback():
    print('Logout')
    itchat.auto_login(hotReload=True, loginCallback=login_callback, exitCallback=logout_callback)

# login
itchat.auto_login(hotReload=True, loginCallback=login_callback, exitCallback=logout_callback)


##
# send message to user by nickname
#
def send_msg_to_person(msg, nickname):

    # search user by nickname
    username = itchat.search_friends(nickName=nickname)[0]
    print username

    # send message to user
    username.send(msg)
    print "message sent to " + username['NickName']


##
# send message to group by nickname
#
def send_msg_to_group(msg, nickname):

    # search group by nickname
    username = itchat.search_chatrooms(name=nickname)[0]
    print username

    # send message to group
    username.send(msg)
    print "message sent to " + username['NickName']


##
# get fortune message
#
def get_english_fortune():
    output = subprocess.check_output(["/usr/games/fortune", "magoosh_basic", "magoosh_common", "magoosh_adv"])
    return output


# main
if __name__ == "__main__":
    app.run()







