/**
 * CNC WebSocket Connection Manager
 * Provides a modular WebSocket interface for CNC communication
 */
class CncWebSocket {
    constructor(url = 'ws://localhost:8080') {
        this.url = url;
        this.ws = null;
        this.connected = false;
        this.messageCallbacks = new Map();
        this.messageId = 0;
        this.eventListeners = new Map();

        // Connection retry settings
        this.maxRetries = 5;
        this.retryDelay = 1000;
        this.currentRetries = 0;
    }

    /**
     * Initialize WebSocket connection
     * @returns {Promise<void>}
     */
    async connect() {
        return new Promise((resolve, reject) => {
            this.ws = new WebSocket(this.url);

            this.ws.onopen = () => {
                console.log('[CncWS] Connected to WebSocket server');
                this.connected = true;
                this.currentRetries = 0;
                this.emit('connected');
                resolve();
            };

            this.ws.onmessage = (event) => {
                try {
                    const response = JSON.parse(event.data);
                    console.log('[CncWS] Message received:', response);

                    // Handle response with callback if available
                    if (response.id && this.messageCallbacks.has(response.id)) {
                        const callback = this.messageCallbacks.get(response.id);
                        this.messageCallbacks.delete(response.id);
                        callback.resolve(response);
                    }

                    // Emit message event for general listeners
                    this.emit('message', response);
                } catch (error) {
                    console.error('[CncWS] Failed to parse message:', error);
                    this.emit('error', error);
                }
            };

            this.ws.onclose = () => {
                console.log('[CncWS] WebSocket connection closed');
                this.connected = false;
                this.emit('disconnected');

                // Auto-reconnect if needed
                if (this.currentRetries < this.maxRetries) {
                    this.currentRetries++;
                    setTimeout(() => {
                        console.log(`[CncWS] Reconnecting... (${this.currentRetries}/${this.maxRetries})`);
                        this.connect();
                    }, this.retryDelay);
                }
            };

            this.ws.onerror = (error) => {
                console.error('[CncWS] WebSocket error:', error);
                this.connected = false;
                this.emit('error', error);
                reject(error);
            };
        });
    }

    /**
     * Send a message and wait for response
     * @param {Object} payload - Message payload
     * @param {number} timeout - Request timeout in ms
     * @returns {Promise<Object>}
     */
    async sendMessage(payload, timeout = 5000) {
        return new Promise((resolve, reject) => {
            if (!this.connected || !this.ws) {
                reject(new Error('WebSocket not connected'));
                return;
            }

            const currentId = ++this.messageId;
            payload.id = currentId;

            // Store callback for response
            this.messageCallbacks.set(currentId, { resolve, reject });

            // Set timeout for request
            setTimeout(() => {
                if (this.messageCallbacks.has(currentId)) {
                    this.messageCallbacks.delete(currentId);
                    reject(new Error('Request timeout'));
                }
            }, timeout);

            this.ws.send(JSON.stringify(payload));
        });
    }

    /**
     * Read CNC object
     * @param {number} thread - Thread ID
     * @param {number} group - Group ID
     * @param {number} offset - Offset ID
     * @param {number} length - Optional length
     * @param {number} datatype - Optional datatype
     * @returns {Promise<Object>}
     */
    async read(thread, group, offset, length = null, datatype = null) {
        const startTime = performance.now();
        const payload = {
            type: 'read',
            thread,
            group,
            offset
        };

        if (length) payload.length = length;
        if (datatype !== null) payload.datatype = datatype;

        try {
            const result = await this.sendMessage(payload);
            const endTime = performance.now();
            const duration = endTime - startTime;

            console.log(`[CncWS-TIMING] READ(${thread},${group},${offset}) -> ${duration.toFixed(2)}ms`);
            return result;
        } catch (error) {
            console.error(`[CncWS-ERROR] READ(${thread},${group},${offset}) failed:`, error);
            return { success: false, error: error.message };
        }
    }

    /**
     * Write CNC object
     * @param {number} thread - Thread ID
     * @param {number} group - Group ID
     * @param {number} offset - Offset ID
     * @param {*} value - Value to write
     * @param {number} datatype - Optional datatype
     * @returns {Promise<Object>}
     */
    async write(thread, group, offset, value, datatype = null) {
        const startTime = performance.now();
        const payload = {
            type: 'write',
            thread,
            group,
            offset,
            value
        };

        if (datatype !== null) payload.datatype = datatype;

        try {
            const result = await this.sendMessage(payload);
            const endTime = performance.now();
            const duration = endTime - startTime;

            console.log(`[CncWS-TIMING] WRITE(${thread},${group},${offset},${JSON.stringify(value)}) -> ${duration.toFixed(2)}ms`);
            return result;
        } catch (error) {
            console.error(`[CncWS-ERROR] WRITE(${thread},${group},${offset}) failed:`, error);
            return { success: false, error: error.message };
        }
    }

    /**
     * Disconnect from WebSocket
     */
    disconnect() {
        if (this.ws) {
            this.ws.close();
            this.ws = null;
        }
        this.connected = false;
    }

    /**
     * Add event listener
     * @param {string} event - Event name
     * @param {function} callback - Callback function
     */
    on(event, callback) {
        if (!this.eventListeners.has(event)) {
            this.eventListeners.set(event, []);
        }
        this.eventListeners.get(event).push(callback);
    }

    /**
     * Remove event listener
     * @param {string} event - Event name
     * @param {function} callback - Callback function
     */
    off(event, callback) {
        if (this.eventListeners.has(event)) {
            const listeners = this.eventListeners.get(event);
            const index = listeners.indexOf(callback);
            if (index > -1) {
                listeners.splice(index, 1);
            }
        }
    }

    /**
     * Emit event
     * @param {string} event - Event name
     * @param {*} data - Event data
     */
    emit(event, data) {
        if (this.eventListeners.has(event)) {
            this.eventListeners.get(event).forEach(callback => {
                try {
                    callback(data);
                } catch (error) {
                    console.error(`[CncWS] Event listener error for ${event}:`, error);
                }
            });
        }
    }

    /**
     * Check if connected
     * @returns {boolean}
     */
    isConnected() {
        return this.connected;
    }
}

// Data type constants for CNC communication
// CNC Data Types - Must match the C enum in cnc-wrapper.h exactly!
const CncDataTypes = {
    NONE: 0,        // CNC_TYPE_NONE
    BOOLEAN: 1,     // CNC_TYPE_BOOLEAN
    UNS08: 2,       // CNC_TYPE_UNS08
    SGN08: 3,       // CNC_TYPE_SGN08
    UNS16: 4,       // CNC_TYPE_UNS16
    SGN16: 5,       // CNC_TYPE_SGN16
    UNS32: 6,       // CNC_TYPE_UNS32
    SGN32: 7,       // CNC_TYPE_SGN32
    UNS64: 8,       // CNC_TYPE_UNS64
    SGN64: 9,       // CNC_TYPE_SGN64
    REAL64: 10,     // CNC_TYPE_REAL64 ← CORRECT VALUE!
    STRUCT: 11,     // CNC_TYPE_STRUCT
    REAL32: 12,     // CNC_TYPE_REAL32
    CHAR: 13,       // CNC_TYPE_CHAR
    STRING: 14,     // CNC_TYPE_STRING
    ERROR: 99       // CNC_TYPE_ERROR
};

// Export for use in modules or global scope
if (typeof module !== 'undefined' && module.exports) {
    module.exports = { CncWebSocket, CncDataTypes };
} else {
    window.CncWebSocket = CncWebSocket;
    window.CncDataTypes = CncDataTypes;
}