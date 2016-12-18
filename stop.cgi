#!/bin/sh

pkill -SIGKILL windUp.cgi;
pkill -SIGKILL windDown.cgi;
pkill -SIGKILL lock.cgi;
pkill -SIGKILL release.cgi;
