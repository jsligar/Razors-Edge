/**
 * Razors Edge Dashboard
 * Modern Electric Vehicle Controller Interface
 */

// ========================================
// Configuration & State
// ========================================
const CONFIG = {
    API_BASE: '',
    WS_RECONNECT_DELAY: 3000,
    UPDATE_INTERVAL: 1000,
    TOAST_DURATION: 5000,
    MAP_DEFAULT_ZOOM: 15
};

const STATE = {
    connected: false,
    authenticated: false,
    lastUpdate: 0,
    websocket: null,
    map: null,
    marker: null,
    telemetry: {},
    geofences: []
};

// ========================================
// Authentication
// ========================================
function checkAuth() {
    const token = localStorage.getItem('auth_token');
    if (!token) {
        window.location.href = '/login.html';
        return false;
    }
    STATE.authenticated = true;
    return true;
}

function logout() {
    localStorage.removeItem('auth_token');
    localStorage.removeItem('username');
    window.location.href = '/login.html';
}

function getAuthHeaders() {
    const token = localStorage.getItem('auth_token');
    return {
        'Content-Type': 'application/json',
        'X-Auth-Token': token
    };
}

// ========================================
// API Client
// ========================================
const API = {
    async request(endpoint, options = {}) {
        try {
            const response = await fetch(CONFIG.API_BASE + endpoint, {
                ...options,
                headers: {
                    ...getAuthHeaders(),
                    ...options.headers
                }
            });

            if (response.status === 401) {
                showToast('Session expired. Please login again.', 'error');
                logout();
                return null;
            }

            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }

            return await response.json();
        } catch (error) {
            console.error('API Error:', error);
            showToast(`API Error: ${error.message}`, 'error');
            return null;
        }
    },

    async getTelemetry() {
        return await this.request('/api/telemetry');
    },

    async getStatus() {
        return await this.request('/api/status');
    },

    async emergencyStop() {
        return await this.request('/api/emergency-stop', { method: 'POST' });
    },

    async setGear(gear) {
        return await this.request('/api/set-gear', {
            method: 'POST',
            body: JSON.stringify({ gear })
        });
    },

    async resetTrip() {
        return await this.request('/api/reset-trip', { method: 'POST' });
    },

    async setHome() {
        return await this.request('/api/set-home', { method: 'POST' });
    },

    async calibration() {
        return await this.request('/api/calibration', { method: 'POST' });
    },

    async getGeofences() {
        return await this.request('/api/geofences');
    }
};

// ========================================
// WebSocket Connection
// ========================================
function connectWebSocket() {
    const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
    const wsUrl = `${protocol}//${window.location.host}/ws`;

    try {
        STATE.websocket = new WebSocket(wsUrl);

        STATE.websocket.onopen = () => {
            console.log('WebSocket connected');
            updateConnectionStatus(true);
            showToast('Connected to controller', 'success');
        };

        STATE.websocket.onmessage = (event) => {
            try {
                const data = JSON.parse(event.data);
                handleWebSocketMessage(data);
            } catch (error) {
                console.error('WebSocket message error:', error);
            }
        };

        STATE.websocket.onerror = (error) => {
            console.error('WebSocket error:', error);
            updateConnectionStatus(false);
        };

        STATE.websocket.onclose = () => {
            console.log('WebSocket closed');
            updateConnectionStatus(false);

            // Attempt reconnect
            setTimeout(() => {
                if (STATE.authenticated) {
                    console.log('Attempting WebSocket reconnect...');
                    connectWebSocket();
                }
            }, CONFIG.WS_RECONNECT_DELAY);
        };
    } catch (error) {
        console.error('WebSocket connection failed:', error);
        updateConnectionStatus(false);

        // Fallback to polling
        startPolling();
    }
}

function handleWebSocketMessage(data) {
    if (data.type === 'telemetry') {
        STATE.telemetry = data.data;
        updateDashboard(data.data);
    } else if (data.type === 'event') {
        handleEvent(data.event, data.data);
    }
}

function handleEvent(eventType, eventData) {
    switch (eventType) {
        case 'emergency_stop':
            showToast('⚠️ EMERGENCY STOP ACTIVATED', 'error');
            break;
        case 'gear_change':
            showToast(`Gear changed to ${eventData.gear}`, 'info');
            break;
        case 'geofence_violation':
            showToast(`Geofence violation: ${eventData.message}`, 'warning');
            break;
        case 'low_battery':
            showToast('⚠️ Low battery warning!', 'warning');
            break;
        default:
            console.log('Unknown event:', eventType, eventData);
    }
}

// ========================================
// Polling Fallback
// ========================================
let pollingInterval = null;

function startPolling() {
    if (pollingInterval) return;

    console.log('Starting polling mode');
    pollingInterval = setInterval(async () => {
        const telemetry = await API.getTelemetry();
        if (telemetry) {
            STATE.telemetry = telemetry;
            updateDashboard(telemetry);
        }
    }, CONFIG.UPDATE_INTERVAL);
}

function stopPolling() {
    if (pollingInterval) {
        clearInterval(pollingInterval);
        pollingInterval = null;
    }
}

// ========================================
// Dashboard Updates
// ========================================
function updateDashboard(data) {
    STATE.lastUpdate = Date.now();

    // Power System
    updateElement('batteryVoltage', formatNumber(data.batteryVoltage, 1));
    updateElement('batteryCurrent', formatNumber(data.batteryCurrent, 1));
    updateElement('batteryPower', formatNumber(data.batteryPower, 0));
    updateElement('batterySOC', formatNumber(data.batterySOC, 0));

    // Battery bar
    const batteryFill = document.getElementById('batteryFill');
    if (batteryFill) {
        batteryFill.style.width = `${data.batterySOC}%`;
        batteryFill.className = `battery-fill ${data.batterySOC < 20 ? 'low' : ''}`;
    }

    // Motor Control
    const gearNames = ['P', '1', '2', '3', 'E', 'S+'];
    updateElement('currentGear', gearNames[data.currentGear] || 'P');
    updateElement('leftMotorSpeed', formatNumber(data.currentSpeedLeft, 0));
    updateElement('rightMotorSpeed', formatNumber(data.currentSpeedRight, 0));
    updateElement('leftMotorCurrent', `${formatNumber(data.motorCurrentLeft, 1)} A`);
    updateElement('rightMotorCurrent', `${formatNumber(data.motorCurrentRight, 1)} A`);

    // Update gear selector
    document.querySelectorAll('.gear-btn').forEach(btn => {
        btn.classList.toggle('active', parseInt(btn.dataset.gear) === data.currentGear);
    });

    // GPS & Navigation
    const gpsStatus = document.getElementById('gpsStatus');
    if (gpsStatus) {
        gpsStatus.textContent = data.gpsFixed ? 'Fixed' : 'Searching';
        gpsStatus.className = `badge ${data.gpsFixed ? 'badge-success' : 'badge-warning'}`;
    }

    updateElement('satelliteCount', `${data.satellites} sats`);
    updateElement('gpsSpeed', formatNumber(data.gpsSpeed, 1));
    updateElement('odometer', formatNumber(data.odometer, 1));
    updateElement('tripMeter', formatNumber(data.tripMeter, 2));
    updateElement('coordinates', `${formatNumber(data.latitude, 5)}, ${formatNumber(data.longitude, 5)}`);

    // Update map
    if (data.gpsFixed && data.latitude && data.longitude) {
        updateMapPosition(data.latitude, data.longitude);
    }

    // Geofencing
    updateElement('safeZone', data.inSafeZone ? 'YES' : 'NO');
    updateElement('speedLimit', formatNumber(data.currentSpeedLimit, 0));
    updateElement('distanceHome', formatNumber(data.distanceToHome, 0));
    updateElement('violations', data.totalViolations || 0);

    // System Status
    updateElement('keySwitch', data.keySwitch ? 'ON' : 'OFF');
    const safetyStatus = document.getElementById('safetyStatus');
    if (safetyStatus) {
        safetyStatus.textContent = data.safetyFault ? 'FAULT' : 'OK';
        safetyStatus.style.color = data.safetyFault ? 'var(--danger)' : 'var(--success)';
    }

    updateElement('uptime', formatUptime(data.uptime));

    // Update free heap if available
    if (data.freeHeap !== undefined) {
        updateElement('freeHeap', formatBytes(data.freeHeap));
    }
}

function updateElement(id, text) {
    const element = document.getElementById(id);
    if (element) {
        element.textContent = text;
    }
}

function updateConnectionStatus(connected) {
    STATE.connected = connected;
    const statusDot = document.querySelector('.status-dot');
    const statusText = document.querySelector('.status-text');

    if (statusDot) {
        statusDot.className = `status-dot ${connected ? 'connected' : 'disconnected'}`;
    }

    if (statusText) {
        statusText.textContent = connected ? 'Connected' : 'Disconnected';
    }
}

// ========================================
// Map Integration
// ========================================
function initializeMap() {
    try {
        STATE.map = L.map('map').setView([0, 0], CONFIG.MAP_DEFAULT_ZOOM);

        L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
            attribution: '© OpenStreetMap contributors',
            maxZoom: 19
        }).addTo(STATE.map);

        // Create custom marker icon
        const vehicleIcon = L.divIcon({
            className: 'vehicle-marker',
            html: '<div style="background: var(--primary); width: 20px; height: 20px; border-radius: 50%; border: 3px solid white; box-shadow: 0 2px 4px rgba(0,0,0,0.3);"></div>',
            iconSize: [20, 20],
            iconAnchor: [10, 10]
        });

        STATE.marker = L.marker([0, 0], { icon: vehicleIcon }).addTo(STATE.map);

    } catch (error) {
        console.error('Map initialization error:', error);
        showToast('Map unavailable - GPS disabled', 'warning');
    }
}

function updateMapPosition(lat, lng) {
    if (!STATE.map || !STATE.marker) return;

    STATE.marker.setLatLng([lat, lng]);

    // Only pan to location on first fix or if far away
    const center = STATE.map.getCenter();
    const distance = STATE.map.distance(center, [lat, lng]);

    if (distance > 100) { // More than 100m away
        STATE.map.setView([lat, lng], STATE.map.getZoom());
    }
}

// ========================================
// Toast Notifications
// ========================================
function showToast(message, type = 'info') {
    const container = document.getElementById('toastContainer');
    if (!container) return;

    const toast = document.createElement('div');
    toast.className = `toast ${type}`;
    toast.innerHTML = `
        <div>${message}</div>
    `;

    container.appendChild(toast);

    setTimeout(() => {
        toast.style.animation = 'slideIn 0.3s ease reverse';
        setTimeout(() => toast.remove(), 300);
    }, CONFIG.TOAST_DURATION);
}

// ========================================
// Event Handlers
// ========================================
function setupEventHandlers() {
    // Emergency Stop
    document.getElementById('emergencyStopBtn')?.addEventListener('click', async () => {
        if (confirm('⚠️ Activate EMERGENCY STOP?')) {
            const result = await API.emergencyStop();
            if (result) {
                showToast('Emergency stop activated', 'error');
            }
        }
    });

    // Gear Selection
    document.querySelectorAll('.gear-btn').forEach(btn => {
        btn.addEventListener('click', async () => {
            const gear = parseInt(btn.dataset.gear);
            const gearNames = ['park', '1st', '2nd', '3rd', 'eco', 'sport'];
            const result = await API.setGear(gearNames[gear]);
            if (result) {
                showToast(`Gear set to ${btn.textContent}`, 'success');
            }
        });
    });

    // Set Home
    document.getElementById('setHomeBtn')?.addEventListener('click', async () => {
        if (!STATE.telemetry.gpsFixed) {
            showToast('GPS not fixed - cannot set home', 'warning');
            return;
        }

        if (confirm('Set current location as home?')) {
            const result = await API.setHome();
            if (result) {
                showToast('Home position set', 'success');
            }
        }
    });

    // Reset Trip
    document.getElementById('resetTripBtn')?.addEventListener('click', async () => {
        if (confirm('Reset trip meter?')) {
            const result = await API.resetTrip();
            if (result) {
                showToast('Trip meter reset', 'success');
            }
        }
    });

    // Calibration
    document.getElementById('calibrationBtn')?.addEventListener('click', async () => {
        if (confirm('Activate OLED calibration screen?')) {
            const result = await API.calibration();
            if (result) {
                showToast('📺 OLED calibration activated!', 'info');
            }
        }
    });

    // Geofence Management
    document.getElementById('manageGeofencesBtn')?.addEventListener('click', () => {
        openGeofenceModal();
    });

    document.getElementById('closeGeofenceModal')?.addEventListener('click', () => {
        closeGeofenceModal();
    });

    // Refresh
    document.getElementById('refreshBtn')?.addEventListener('click', async () => {
        showToast('Refreshing...', 'info');
        const telemetry = await API.getTelemetry();
        if (telemetry) {
            STATE.telemetry = telemetry;
            updateDashboard(telemetry);
            showToast('Dashboard refreshed', 'success');
        }
    });

    // Logout
    document.getElementById('logoutBtn')?.addEventListener('click', () => {
        if (confirm('Logout from dashboard?')) {
            logout();
        }
    });

    // Settings (placeholder)
    document.getElementById('settingsBtn')?.addEventListener('click', () => {
        showToast('Settings coming soon', 'info');
    });
}

// ========================================
// Geofence Management
// ========================================
function openGeofenceModal() {
    const modal = document.getElementById('geofenceModal');
    if (modal) {
        modal.classList.add('active');
        loadGeofences();
    }
}

function closeGeofenceModal() {
    const modal = document.getElementById('geofenceModal');
    if (modal) {
        modal.classList.remove('active');
    }
}

async function loadGeofences() {
    const list = document.getElementById('geofenceList');
    if (!list) return;

    list.innerHTML = '<p class="text-muted">Loading geofences...</p>';

    const geofences = await API.getGeofences();

    if (!geofences || geofences.length === 0) {
        list.innerHTML = '<p class="text-muted">No geofences configured</p>';
        return;
    }

    STATE.geofences = geofences;

    list.innerHTML = geofences.map((fence, index) => `
        <div class="geofence-item" style="background: var(--bg-input); padding: var(--spacing-md); border-radius: var(--border-radius-sm); margin-bottom: var(--spacing-sm);">
            <div style="display: flex; justify-content: space-between; align-items: start;">
                <div>
                    <strong>${fence.name}</strong>
                    <div style="font-size: 0.875rem; color: var(--text-secondary);">
                        Type: ${fence.type} | Radius: ${fence.radius}m
                    </div>
                </div>
                <button class="btn btn-danger btn-sm" onclick="removeGeofence(${index})">Remove</button>
            </div>
        </div>
    `).join('');
}

function removeGeofence(index) {
    if (confirm('Remove this geofence?')) {
        showToast('Geofence removal coming soon', 'info');
        // Implement actual removal logic
    }
}

// ========================================
// Utility Functions
// ========================================
function formatNumber(value, decimals = 0) {
    if (value === null || value === undefined || isNaN(value)) return '--';
    return Number(value).toFixed(decimals);
}

function formatUptime(ms) {
    if (!ms) return '--';
    const seconds = Math.floor(ms / 1000);
    const minutes = Math.floor(seconds / 60);
    const hours = Math.floor(minutes / 60);
    const days = Math.floor(hours / 24);

    if (days > 0) return `${days}d ${hours % 24}h`;
    if (hours > 0) return `${hours}h ${minutes % 60}m`;
    return `${minutes}m ${seconds % 60}s`;
}

function formatBytes(bytes) {
    if (!bytes) return '--';
    if (bytes < 1024) return `${bytes} B`;
    if (bytes < 1024 * 1024) return `${(bytes / 1024).toFixed(1)} KB`;
    return `${(bytes / (1024 * 1024)).toFixed(1)} MB`;
}

// ========================================
// Initialization
// ========================================
function initialize() {
    console.log('Initializing Razors Edge Dashboard...');

    // Check authentication
    if (!checkAuth()) return;

    // Display username
    const username = localStorage.getItem('username');
    console.log(`Logged in as: ${username}`);

    // Initialize map
    initializeMap();

    // Setup event handlers
    setupEventHandlers();

    // Connect WebSocket (with fallback to polling)
    connectWebSocket();

    // Initial data fetch
    API.getTelemetry().then(telemetry => {
        if (telemetry) {
            STATE.telemetry = telemetry;
            updateDashboard(telemetry);
        }
    });

    // Show welcome message
    showToast(`Welcome, ${username || 'User'}!`, 'success');

    console.log('Dashboard initialized');
}

// Start when DOM is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', initialize);
} else {
    initialize();
}

// Cleanup on page unload
window.addEventListener('beforeunload', () => {
    if (STATE.websocket) {
        STATE.websocket.close();
    }
    stopPolling();
});

// Global error handler
window.addEventListener('error', (event) => {
    console.error('Global error:', event.error);
    showToast('An error occurred', 'error');
});

// Make some functions globally accessible
window.removeGeofence = removeGeofence;
