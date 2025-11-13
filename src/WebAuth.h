#ifndef WEB_AUTH_H
#define WEB_AUTH_H

#include <Arduino.h>
#include <WebServer.h>
#include <Preferences.h>
#include "Logger.h"

// Simple session management
struct AuthSession {
    String token;
    unsigned long createdAt;
    unsigned long lastActivity;
    IPAddress clientIP;
    bool isValid;
};

class WebAuth {
public:
    static WebAuth& getInstance() {
        static WebAuth instance;
        return instance;
    }

    // Initialize authentication system
    bool init() {
        preferences.begin("web-auth", false);

        // Load or create default credentials
        username = preferences.getString("username", "admin");
        String storedHash = preferences.getString("password_hash", "");

        if (storedHash.length() == 0) {
            // First run - set default password
            setPassword("RazorEdge2025");
            LOG_WARN("Auth", "Using default password - please change!");
        } else {
            passwordHash = storedHash;
        }

        sessionTimeout = preferences.getUInt("session_timeout", 1800000); // 30 min default

        LOG_INFO("Auth", "Authentication system initialized");
        return true;
    }

    // Check if authentication is required
    bool isAuthRequired() {
        return authEnabled;
    }

    // Enable/disable authentication
    void setAuthEnabled(bool enabled) {
        authEnabled = enabled;
        preferences.putBool("auth_enabled", enabled);
        LOG_INFO("Auth", "Authentication %s", enabled ? "enabled" : "disabled");
    }

    // Authenticate user and create session
    bool authenticate(const String& user, const String& pass, IPAddress clientIP) {
        if (user != username) {
            LOG_WARN("Auth", "Authentication failed - invalid username from %s", clientIP.toString().c_str());
            return false;
        }

        String hash = hashPassword(pass);
        if (hash != passwordHash) {
            LOG_WARN("Auth", "Authentication failed - invalid password from %s", clientIP.toString().c_str());
            return false;
        }

        // Create new session
        currentSession.token = generateToken();
        currentSession.createdAt = millis();
        currentSession.lastActivity = millis();
        currentSession.clientIP = clientIP;
        currentSession.isValid = true;

        LOG_INFO("Auth", "Authentication successful from %s", clientIP.toString().c_str());
        return true;
    }

    // Validate session token
    bool validateSession(const String& token, IPAddress clientIP) {
        if (!authEnabled) {
            return true; // Auth disabled, allow all
        }

        if (!currentSession.isValid) {
            return false;
        }

        if (token != currentSession.token) {
            return false;
        }

        if (clientIP != currentSession.clientIP) {
            LOG_WARN("Auth", "Session token used from different IP");
            return false;
        }

        // Check timeout
        unsigned long now = millis();
        if (now - currentSession.lastActivity > sessionTimeout) {
            LOG_INFO("Auth", "Session expired for %s", clientIP.toString().c_str());
            currentSession.isValid = false;
            return false;
        }

        // Update activity
        currentSession.lastActivity = now;
        return true;
    }

    // Logout (invalidate session)
    void logout() {
        currentSession.isValid = false;
        LOG_INFO("Auth", "Session logged out");
    }

    // Change password
    bool setPassword(const String& newPassword) {
        if (newPassword.length() < 8) {
            LOG_ERROR("Auth", "Password too short (minimum 8 characters)");
            return false;
        }

        passwordHash = hashPassword(newPassword);
        preferences.putString("password_hash", passwordHash);

        LOG_INFO("Auth", "Password changed successfully");
        return true;
    }

    // Change username
    bool setUsername(const String& newUsername) {
        if (newUsername.length() < 3) {
            LOG_ERROR("Auth", "Username too short (minimum 3 characters)");
            return false;
        }

        username = newUsername;
        preferences.putString("username", username);

        LOG_INFO("Auth", "Username changed to: %s", username.c_str());
        return true;
    }

    // Get current session token
    String getSessionToken() const {
        return currentSession.token;
    }

    // Set session timeout (milliseconds)
    void setSessionTimeout(unsigned long timeoutMs) {
        sessionTimeout = timeoutMs;
        preferences.putUInt("session_timeout", timeoutMs);
    }

    // Helper for WebServer integration
    bool checkAuth(WebServer& server) {
        if (!authEnabled) {
            return true;
        }

        // Check for session token in cookie or header
        String token = "";

        if (server.hasHeader("X-Auth-Token")) {
            token = server.header("X-Auth-Token");
        } else if (server.hasHeader("Cookie")) {
            String cookies = server.header("Cookie");
            int tokenPos = cookies.indexOf("auth_token=");
            if (tokenPos >= 0) {
                int endPos = cookies.indexOf(';', tokenPos);
                if (endPos < 0) endPos = cookies.length();
                token = cookies.substring(tokenPos + 11, endPos);
            }
        }

        IPAddress clientIP = server.client().remoteIP();

        if (token.length() > 0 && validateSession(token, clientIP)) {
            return true;
        }

        // Not authenticated - send 401
        server.send(401, "application/json", "{\"error\":\"Authentication required\"}");
        return false;
    }

private:
    WebAuth() :
        authEnabled(true),
        sessionTimeout(1800000) // 30 minutes
    {}

    ~WebAuth() {
        preferences.end();
    }

    // Prevent copying
    WebAuth(const WebAuth&) = delete;
    WebAuth& operator=(const WebAuth&) = delete;

    Preferences preferences;
    String username;
    String passwordHash;
    bool authEnabled;
    unsigned long sessionTimeout;
    AuthSession currentSession;

    // Simple password hashing (for embedded use - not cryptographically secure)
    // For production, consider using a proper crypto library
    String hashPassword(const String& password) {
        // Simple XOR-based hash with salt (better than plaintext)
        // NOTE: For production, use proper crypto like mbedTLS
        const char* salt = "RazorsEdge2025Salt";
        String salted = password + salt;
        uint32_t hash = 0x12345678;

        for (size_t i = 0; i < salted.length(); i++) {
            hash = ((hash << 5) + hash) ^ salted[i];
        }

        char hashStr[16];
        sprintf(hashStr, "%08X", hash);
        return String(hashStr);
    }

    // Generate random session token
    String generateToken() {
        char token[32];
        for (int i = 0; i < 31; i++) {
            token[i] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"[random(62)];
        }
        token[31] = '\0';
        return String(token);
    }
};

#endif // WEB_AUTH_H
