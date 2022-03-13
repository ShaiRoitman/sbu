FROM ubuntu:18.04

RUN apt update
RUN apt install -y python3
RUN rm -rf /var/lib/apt/lists/*

COPY --from=webservice_venv:1.0.0 /venv /venv
ENV PATH $PATH:/venv/bin

RUN mkdir /sbu
RUN mkdir /sbu/App
RUN mkdir /sbu/App/sbu
RUN mkdir /sbu/App/sbu/so
RUN mkdir /sbu/App/SBUWebApp
RUN mkdir /sbu/Configuration
RUN mkdir /sbu/Metadata
RUN mkdir /sbu/Workdir
RUN mkdir /sbu/Restore
RUN mkdir /mounts

RUN mkdir /work

WORKDIR /sbu

COPY ./sbu_app /sbu/App
COPY ./SBUWebApp /sbu/App/SBUWebApp
COPY ./start.sh /sbu/start.sh
ENV LD_LIBRARY_PATH $LD_LIBRARY_PATH:/sbu/App/so
ENV PATH $PATH:/sbu/App

COPY DefaultSBUApp.config /sbu/App
COPY DefaultSBUWebAppConfig.json /sbu/App/SBUWebApp/

WORKDIR /sbu

EXPOSE 5000

ENV SBU_CONFIG_PATH /sbu/Configuration
ENV SBU_CONFIG $SBU_CONFIG_PATH/SBUApp.config
ENV SBUWEBAPP_CONFIG $SBU_CONFIG_PATH/SBUWebAppConfig.json

COPY dockerEntryPoint.sh /sbu

CMD /sbu/dockerEntryPoint.sh



