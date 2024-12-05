FROM nodered/node-red:latest
USER root
RUN npm install node-red-contrib-coap
RUN npm install node-red-dashboard
RUN npm install node-red-node-mysql
RUN npm install node-red-contrib-influxdb
RUN npm install @meowwolf/node-red-contrib-amqp
RUN npm install node-red-node-xmpp
RUN npm install node-red-contrib-telegrambot
USER node-red
