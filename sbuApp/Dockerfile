FROM ubuntu:18.04

RUN apt update
RUN apt install -y python3 python3-pip
RUN apt install -y python3-venv

RUN mkdir /web_sbu

WORKDIR /web_sbu

RUN python3 -m venv /web_sbu /web_sbu/sbu_venv

COPY SBUWebApp/requirements.txt /sbu/App/SBUWebApp/requirements.txt
RUN  sbu_venv/bin/python3 -m pip install -r /sbu/App/SBUWebApp/requirements.txt

CMD /bin/bash

