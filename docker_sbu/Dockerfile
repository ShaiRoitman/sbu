FROM ubuntu:18.04

RUN apt update
RUN apt install -y python3
RUN apt install -y cifs-utils
RUN rm -rf /var/lib/apt/lists/*

RUN mkdir /sbu
RUN mkdir /sbu/App
RUN mkdir /sbu/App/sbu
RUN mkdir /sbu/App/sbu/so
RUN mkdir /sbu/App/SBUWebApp
RUN mkdir /sbu/Configuration
RUN mkdir /sbu/Metadata
RUN mkdir /sbu/Work
RUN mkdir /sbu/Restore

RUN mkdir /work

COPY sbu_app/so/* /sbu/App/sbu/so/
COPY sbu_app/sbu /sbu/App/sbu/

ENV LD_LIBRARY_PATH $LD_LIBRARY_PATH:/sbu/App/sbu/so
ENV PATH $PATH:/sbu/App/sbu

WORKDIR /work

CMD /bin/bash
