/*global require, module, global*/
'use strict';
require = require('module')._load('./AgentLoader');

var internalIO = require('./internalIO/build/Release/internalIO');
var InternalIn = internalIO.In;
var InternalOut = internalIO.Out;

var avstream = require('./avstreamLib/build/Release/avstream');
var AVStreamIn = avstream.AVStreamIn;
var AVStreamOut = avstream.AVStreamOut;
var logger = require('./logger').logger;
var path = require('path');
var Connections = require('./connections');

// Logger
var log = logger.getLogger('AvstreamNode');

var InternalConnectionFactory = require('./InternalConnectionFactory');

module.exports = function (rpcClient) {
    var that = {};
    var connections = new Connections;
    var internalConnFactory = new InternalConnectionFactory;

    var notifyStatus = (controller, sessionId, direction, status) => {
        rpcClient.remoteCast(controller, 'onSessionProgress', [sessionId, direction, status]);
    };

    var createAVStreamIn = function (sessionId, options) {
        var avstream_options = {type: 'avstream',
                                has_audio: !!options.media.audio,//FIXME: maybe 'auto'
                                has_video: !!options.media.video,//FIXME: maybe 'auto'
                                transport: options.connection.transportProtocol,
                                buffer_size: options.connection.bufferSize,
                                url: options.connection.url};

        var connection = new AVStreamIn(avstream_options, function (message) {
            log.debug('avstream-in status message:', message);
            notifyStatus(options.controller, sessionId, 'in', JSON.parse(message));
        });

        return connection;
    };

    var createAVStreamOut = function (connectionId, options) {
        var avstream_options = {type: 'avstream',
                                require_audio: !!options.media.audio,
                                require_video: !!options.media.video,
                                audio_codec: 'aac'/*FIXME:hard coded*/,
                                video_codec: 'h264'/*FIXME: hard coded*/,
                                video_resolution: 'vga'/*FIXME: hard coded*/,
                                url: options.connection.url,
                                initializeTimeout: global.config.avstream.initializeTimeout};

        var connection = new AVStreamOut(avstream_options, function (error) {
            if (error) {
                log.error('avstream-out init error:', error);
                notifyStatus(options.controller, connectionId, 'out', {type: 'failed', reason: error});
            } else {
                notifyStatus(options.controller, connectionId, 'out', {type: 'ready'});
            }
        });
        connection.addEventListener('fatal', function (error) {
            if (error) {
                log.error('avstream-out fatal error:', error);
                notifyStatus(options.controller, connectionId, 'out', {type: 'failed', reason: 'avstream_out fatal error: ' + error});
            }
        });

        connection.receiver = function(type) {
            return this;
        };
        return connection;
    };

    var onSuccess = function (callback) {
        return function(result) {
            callback('callback', result);
        };
    };

    var onError = function (callback) {
        return function(reason) {
            if (typeof reason === 'string') {
                callback('callback', 'error', reason);
            } else {
                callback('callback', reason);
            }
        };
    };

    that.createInternalConnection = function (connectionId, direction, internalOpt, callback) {
        internalOpt.minport = global.config.internal.minport;
        internalOpt.maxport = global.config.internal.maxport;
        var portInfo = internalConnFactory.create(connectionId, direction, internalOpt);
        callback('callback', {ip: that.clusterIP, port: portInfo});
    };

    that.destroyInternalConnection = function (connectionId, direction, callback) {
        internalConnFactory.destroy(connectionId, direction);
        callback('callback', 'ok');
    };

    that.publish = function (connectionId, connectionType, options, callback) {
        log.debug('publish, connectionId:', connectionId, 'connectionType:', connectionType, 'options:', options);
        if (connections.getConnection(connectionId)) {
            return callback('callback', {type: 'failed', reason: 'Connection already exists:'+connectionId});
        }

        var conn = null;
        switch (connectionType) {
            case 'internal':
                conn = internalConnFactory.fetch(connectionId, 'in');
                if (conn)
                    conn.connect(options);
                break;
            case 'streaming':
                conn = createAVStreamIn(connectionId, options);
                break;
            default:
                log.error('Connection type invalid:' + connectionType);
        }
        if (!conn) {
            log.error('Create connection failed', connectionId);
            return callback('callback', {type: 'failed', reason: 'Create Connection failed'});
        }

        return connections.addConnection(connectionId, connectionType, options.controller, conn, 'in')
        .then(onSuccess(callback), onError(callback));
    };

    that.unpublish = function (connectionId, callback) {
        log.debug('unpublish, connectionId:', connectionId);
        var conn = connections.getConnection(connectionId);
        connections.removeConnection(connectionId).then(function(ok) {
            if (conn && conn.type === 'internal') {
                internalConnFactory.destroy(connectionId, 'in');
            } else if (conn) {
                conn.connection.close();
            }
            callback('callback', 'ok');
        }, onError(callback));
    };

    that.subscribe = function (connectionId, connectionType, options, callback) {
        log.debug('subscribe, connectionId:', connectionId, 'connectionType:', connectionType, 'options:', options);
        if (connections.getConnection(connectionId)) {
            return callback('callback', {type: 'failed', reason: 'Connection already exists:'+connectionId});
        }

        var conn = null;
        switch (connectionType) {
            case 'internal':
                conn = internalConnFactory.fetch(connectionId, 'out');
                if (conn)
                    conn.connect(options);
                break;
            case 'streaming':
                conn = createAVStreamOut(connectionId, options);
                break;
            default:
                log.error('Connection type invalid:' + connectionType);
        }
        if (!conn) {
            log.error('Create connection failed', connectionId, connectionType);
            return callback('callback', {type: 'failed', reason: 'Create Connection failed'});
        }

        connections.addConnection(connectionId, connectionType, options.controller, conn, 'out')
        .then(onSuccess(callback), onError(callback));
    };

    that.unsubscribe = function (connectionId, callback) {
        log.debug('unsubscribe, connectionId:', connectionId);
        var conn = connections.getConnection(connectionId);
        connections.removeConnection(connectionId).then(function(ok) {
            if (conn && conn.type === 'internal') {
                internalConnFactory.destroy(connectionId, 'out');
            } else if (conn) {
                conn.connection.close();
            }
            callback('callback', 'ok');
        }, onError(callback));
    };

    that.linkup = function (connectionId, audioFrom, videoFrom, callback) {
        connections.linkupConnection(connectionId, audioFrom, videoFrom).then(onSuccess(callback), onError(callback));
    };

    that.cutoff = function (connectionId, callback) {
        connections.cutoffConnection(connectionId).then(onSuccess(callback), onError(callback));
    };

    that.close = function() {
        log.debug('close called');
        var connIds = connections.getIds();
        for (let connectionId of connIds) {
            var conn = connections.getConnection(connectionId);
            connections.removeConnection(connectionId);
            if (conn && conn.type === 'internal') {
                internalConnFactory.destroy(connectionId, conn.direction);
            } else if (conn) {
                conn.connection.close();
            }
        }
    };

    that.onFaultDetected = function (message) {
        connections.onFaultDetected(message);
    };

    return that;
};