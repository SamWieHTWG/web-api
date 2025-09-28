/**
 * CNC Object Definitions and Helper Functions
 * Based on cnc_objects.csv and CNC system documentation
 */

/**
 * CNC Object Registry
 * Maps object names to their thread, group, and offset values
 */
const CncObjects = {
    // Correct number of axes object: Thread=1, Group=131840, Offset=7
    NUMBER_OF_AXES: {
        thread: 1,
        group: 131840,    // 0x20300 in hex
        offset: 7,        // 7 in decimal
        datatype: 'UNS16',
        description: 'number of axes'
    },

    // From cnc_objects.csv line 290: 0x20300;0x100c3;axis name;STRING;16;false;-;AXIS;TASK_IPO
    // Note: offset pattern is 0x<AXIDX>00c3 where AXIDX is axis ID (1-based: 1, 2, 3, ...)
    // Example: Axis 1 = 0x100c3, Axis 2 = 0x200c3, Axis 3 = 0x300c3
    AXIS_NAME_TEMPLATE: {
        thread: 1,
        group: 0x20300,   // 131840 in decimal
        offsetPattern: 0x00c3,  // Base offset, combine with axis ID (1-based)
        datatype: 'STRING',
        length: 16,
        description: 'axis name'
    },

    // MCM (Motion Control Manager) objects for system control
    MCM_ACTIVE: {
        thread: 3,
        group: 0x20101,   // 131329 in decimal
        offset: 0x103,    // 259 in decimal
        datatype: 'NONE',
        description: 'MCM active state'
    },

    // Axis position objects (template - actual addresses depend on axis configuration)
    AXIS_POSITIONS: {
        thread: 3,
        group: 0x20201,   // 131585 in decimal
        offset: 0x1104,   // 4356 in decimal
        datatype: 'REAL64',
        description: 'axis active positions'
    }
};

/**
 * CNC Object Reader
 * High-level interface for reading CNC objects
 */
class CncObjectReader {
    constructor(webSocket) {
        this.ws = webSocket;
    }

    /**
     * Read number of axes from CNC system
     * @returns {Promise<{success: boolean, axisCount?: number, error?: string}>}
     */
    async readAxesCount() {
        try {
            const obj = CncObjects.NUMBER_OF_AXES;
            const result = await this.ws.read(
                obj.thread,
                obj.group,
                obj.offset,
                2, // UNS16 = 2 bytes
                CncDataTypes.UINT16
            );

            if (result.success && result.value !== undefined) {
                return {
                    success: true,
                    axisCount: result.value,
                    raw: result
                };
            } else {
                return {
                    success: false,
                    error: result.error || 'Failed to read axes count',
                    raw: result
                };
            }
        } catch (error) {
            return {
                success: false,
                error: error.message
            };
        }
    }


    /**
     * Read generic CNC object by name
     * @param {string} objectName - Name of the object from CncObjects
     * @param {number} customLength - Optional custom length override
     * @returns {Promise<Object>}
     */
    async readObject(objectName, customLength = null) {
        if (!CncObjects[objectName]) {
            return {
                success: false,
                error: `Unknown CNC object: ${objectName}`
            };
        }

        try {
            const obj = CncObjects[objectName];
            const dataType = CncDataTypes[obj.datatype] || CncDataTypes.NONE;

            // Determine length based on datatype if not provided
            let length = customLength;
            if (!length) {
                const typeSizes = {
                    'UNS16': 2,
                    'UNS32': 4,
                    'REAL64': 8,
                    'STRING': 64 // Default string length
                };
                length = typeSizes[obj.datatype] || null;
            }

            const result = await this.ws.read(
                obj.thread,
                obj.group,
                obj.offset,
                length,
                dataType
            );

            return {
                success: result.success,
                value: result.value,
                objectName,
                description: obj.description,
                error: result.error,
                raw: result
            };
        } catch (error) {
            return {
                success: false,
                error: error.message,
                objectName
            };
        }
    }

    /**
     * Read a CNC object value with explicit parameters
     * @param {number} thread - Thread ID
     * @param {number} group - Group ID
     * @param {number} offset - Offset ID
     * @param {string} datatype - Data type name (e.g., 'REAL64', 'UNS32')
     * @param {number} length - Length in bytes
     * @returns {Promise<{success: boolean, value?: any, error?: string}>}
     */
    async readObjectValue(thread, group, offset, datatype, length) {
        try {
            // Pass datatype as string directly (e.g., "REAL64", "UNS32")
            const result = await this.ws.read(
                thread,
                group,
                offset,
                length,
                datatype  // Send as string instead of enum number
            );

            return {
                success: result.success,
                value: result.value,
                error: result.error,
                raw: result
            };
        } catch (error) {
            return {
                success: false,
                error: error.message
            };
        }
    }

    /**
     * Read axis names for all axes
     * @param {number} axisCount - Number of axes to read names for
     * @returns {Promise<{success: boolean, axisNames?: string[], error?: string}>}
     */
    async readAxisNames(axisCount) {
        try {
            const template = CncObjects.AXIS_NAME_TEMPLATE;
            const axisNames = [];
            const errors = [];

            // Read each axis name
            for (let axisIndex = 0; axisIndex < axisCount; axisIndex++) {
                // Calculate offset: 0x<AXIDX>00c3 where AXIDX is axis index (1-based, not 0-based)
                const axisId = axisIndex + 1;  // Axes start from 1, not 0
                const offset = (axisId << 16) | template.offsetPattern;

                try {
                    const result = await this.ws.read(
                        template.thread,
                        template.group,
                        offset,
                        template.length,
                        CncDataTypes.STRING
                    );

                    if (result.success && result.value) {
                        // Clean up the string (remove null terminators and trim)
                        let axisName = result.value.toString().replace(/\0/g, '').trim();
                        if (!axisName) {
                            axisName = `AXIS${axisId}`;  // Fallback name using 1-based ID
                        }
                        axisNames.push(axisName);
                    } else {
                        axisNames.push(`AXIS${axisId}`);  // Fallback name using 1-based ID
                        errors.push(`Failed to read axis ${axisId}: ${result.error || 'Unknown error'}`);
                    }
                } catch (error) {
                    axisNames.push(`AXIS${axisId}`);  // Fallback name using 1-based ID
                    errors.push(`Error reading axis ${axisId}: ${error.message}`);
                }
            }

            return {
                success: true,
                axisNames,
                errors: errors.length > 0 ? errors : undefined
            };
        } catch (error) {
            return {
                success: false,
                error: error.message
            };
        }
    }
}

/**
 * CNC Status Monitor
 * Manages periodic reading and updating of CNC status
 */
class CncStatusMonitor {
    constructor(webSocket, objectReader) {
        this.ws = webSocket;
        this.reader = objectReader;
        this.intervals = new Map();
        this.callbacks = new Map();
    }

    /**
     * Start monitoring an object
     * @param {string} objectName - CNC object name
     * @param {function} callback - Update callback
     * @param {number} interval - Update interval in ms
     */
    startMonitoring(objectName, callback, interval = 1000) {
        // Stop existing monitoring if any
        this.stopMonitoring(objectName);

        // Store callback
        this.callbacks.set(objectName, callback);

        // Start interval
        const intervalId = setInterval(async () => {
            try {
                const result = await this.reader.readObject(objectName);
                callback(result);
            } catch (error) {
                callback({
                    success: false,
                    error: error.message,
                    objectName
                });
            }
        }, interval);

        this.intervals.set(objectName, intervalId);
        console.log(`[CncMonitor] Started monitoring ${objectName} every ${interval}ms`);

        // Do initial read
        this.reader.readObject(objectName).then(callback);
    }

    /**
     * Stop monitoring an object
     * @param {string} objectName - CNC object name
     */
    stopMonitoring(objectName) {
        if (this.intervals.has(objectName)) {
            clearInterval(this.intervals.get(objectName));
            this.intervals.delete(objectName);
            this.callbacks.delete(objectName);
            console.log(`[CncMonitor] Stopped monitoring ${objectName}`);
        }
    }

    /**
     * Stop all monitoring
     */
    stopAll() {
        for (const objectName of this.intervals.keys()) {
            this.stopMonitoring(objectName);
        }
    }
}

// Export for use in modules or global scope
if (typeof module !== 'undefined' && module.exports) {
    module.exports = { CncObjects, CncObjectReader, CncStatusMonitor };
} else {
    window.CncObjects = CncObjects;
    window.CncObjectReader = CncObjectReader;
    window.CncStatusMonitor = CncStatusMonitor;
}