package generator;

import com.trolltech.qt.*;
import com.trolltech.qt.network.*;

class QAbstractSocket___ extends QAbstractSocket {

    public Signal2<QNetworkProxy, QAuthenticator> proxyAuthenticationRequired = new Signal2<QNetworkProxy, QAuthenticator>();
    private boolean inEmission = false;

    @SuppressWarnings("unused")
    private void emitProxyAuthenticationRequiredPrivate(QNetworkProxy proxy, QAuthenticator authenticator) {
        if (!inEmission) {
            inEmission = true;
            proxyAuthenticationRequiredPrivate.emit(proxy, authenticator.nativePointer());
            inEmission = false;
        }
    }

    @SuppressWarnings("unused")
    private void emitProxyAuthenticationRequired(QNetworkProxy proxy, QNativePointer authenticator) {
        if (!inEmission) {
            inEmission = true;
            proxyAuthenticationRequired.emit(proxy, QAuthenticator.fromNativePointer(authenticator));
            inEmission = false;
        }
    }

    public final void connectToHost(String host, int port, com.trolltech.qt.core.QIODevice.OpenMode mode) {
        connectToHost(host, (char) port, mode);
    }

    public final void connectToHost(QHostAddress host, int port, com.trolltech.qt.core.QIODevice.OpenMode mode) {
        connectToHost(host, (char) port, mode);
    }

    protected final void connectToHostImplementation(String host, int port, com.trolltech.qt.core.QIODevice.OpenMode mode) {
        connectToHostImplementation(host, (char) port, mode);
    }

    public final int localPort() {
        return localPort_private();
    }

    public final int peerPort() {
        return peerPort_private();
    }

    protected final void setLocalPort(int port) {
        setLocalPort((char) port);
    }

    protected final void setPeerPort(int port) {
        setPeerPort((char) port);
    }

}// class

class QHttp___ extends QHttp {

    public Signal2<QNetworkProxy, QAuthenticator> proxyAuthenticationRequired = new Signal2<QNetworkProxy, QAuthenticator>();
    private boolean inEmission = false;

    @SuppressWarnings("unused")
    private void emitProxyAuthenticationRequiredPrivate(QNetworkProxy proxy, QAuthenticator authenticator) {
        if (!inEmission) {
            inEmission = true;
            proxyAuthenticationRequiredPrivate.emit(proxy, authenticator.nativePointer());
            inEmission = false;
        }
    }

    @SuppressWarnings("unused")
    private void emitProxyAuthenticationRequired(QNetworkProxy proxy, QNativePointer authenticator) {
        if (!inEmission) {
            inEmission = true;
            proxyAuthenticationRequired.emit(proxy, QAuthenticator.fromNativePointer(authenticator));
            inEmission = false;
        }
    }

    public Signal3<String, Integer, QAuthenticator> authenticationRequired = new Signal3<String, Integer, QAuthenticator>();
    private boolean inEmissionAuthenticationRequired = false;

    @SuppressWarnings("unused")
    private void emitAuthenticationRequiredPrivate(String hostName, int port, QAuthenticator authenticator) {
        if (!inEmissionAuthenticationRequired) {
            inEmissionAuthenticationRequired = true;
            authenticationRequiredPrivate.emit(hostName, (char) port, authenticator.nativePointer());
            inEmissionAuthenticationRequired = false;
        }
    }

    @SuppressWarnings("unused")
    private void emitAuthenticationRequired(String hostName, char port, QNativePointer authenticator) {
        if (!inEmissionAuthenticationRequired) {
            inEmissionAuthenticationRequired = true;
            authenticationRequired.emit(hostName, (int) port, QAuthenticator.fromNativePointer(authenticator));
            inEmissionAuthenticationRequired = false;
        }
    }

    public QHttp(String hostname, int port) {
        this(hostname, (char) port);
    }

    public QHttp(String hostname, int port, com.trolltech.qt.core.QObject parent) {
        this(hostname, (char) port, parent);
    }

    public QHttp(String hostname, QHttp.ConnectionMode mode, int port) {
        this(hostname, mode, (char) port);
    }

    public QHttp(String hostname, QHttp.ConnectionMode mode, int port, com.trolltech.qt.core.QObject parent) {
        this(hostname, mode, (char) port, parent);
    }

    public final int setHost(String hostname, QHttp.ConnectionMode mode, int port) {
        return setHost(hostname, mode, (char) port);
    }

    public final int setHost(String hostname, int port) {
        return setHost(hostname, (char) port);
    }

}// class

class QTcpServer___ extends QTcpServer {

    public final boolean listen(QHostAddress address, int port) {
        return listen(address, (char) port);
    }

    public final int serverPort() {
        return serverPort_private();
    }

    public enum Result {
        Success, Failure, TimedOut
    }

    public final Result waitForNewConnection(int msec) {
        QNativePointer np = new QNativePointer(QNativePointer.Type.Boolean);
        boolean success = waitForNewConnection(msec, np);

        return (np.booleanValue() ? Result.TimedOut : (success ? Result.Success : Result.Failure));
    }

    public final Result waitForNewConnection() {
        return waitForNewConnection(0);
    }

}// class

class QUdpSocket___ extends QUdpSocket {

    public final boolean bind(QHostAddress address, int port) {
        return bind_private(address, (char) port);
    }

    public final boolean bind(int port) {
        return bind_private((char) port);
    }

    public final boolean bind(QHostAddress address, int port, BindMode mode) {
        return bind_private(address, (char) port, mode);
    }

    public final boolean bind(int port, BindMode mode) {
        return bind_private((char) port, mode);
    }

    public static class HostInfo {
        public HostInfo() {
            address = new QHostAddress();
            port = 0;
        }

        public QHostAddress address;
        public int port;
    }

    public final int readDatagram(byte data[], HostInfo info) {
        QNativePointer np = new QNativePointer(QNativePointer.Type.Byte, data.length);
        QNativePointer address = info != null && info.address != null ? info.address.nativePointer() : null;
        QNativePointer port = new QNativePointer(QNativePointer.Type.Char);

        int len = (int) readDatagram(np, data.length, address, port);
        if (info != null)
            info.port = port.charValue();
        for (int i = 0; i < len; ++i)
            data[i] = np.byteAt(i);

        return len;
    }

    public final int writeDatagram(byte data[], QHostAddress address, int port) {
        QNativePointer np = com.trolltech.qt.QtJambiInternal.byteArrayToNativePointer(data);
        return (int) writeDatagram(np, data.length, address, (char) port);
    }

    public final int writeDatagram(com.trolltech.qt.core.QByteArray data, QHostAddress address, int port) {
        return (int) writeDatagram(data, address, (char) port);
    }

}// class

class QNetworkProxy___ extends QNetworkProxy {

    public QNetworkProxy(QNetworkProxy.ProxyType type, String host, int port) {
        this(type, host, (char) port);
    }

    public QNetworkProxy(QNetworkProxy.ProxyType type, String host, int port, String username) {
        this(type, host, (char) port, username);
    }

    public QNetworkProxy(QNetworkProxy.ProxyType type, String host, int port, String username, String password) {
        this(type, host, (char) port, username, password);
    }

    public final void setPort(int port) {
        setPort((char) port);
    }

    public final int port() {
        return port_private();
    }

}// class