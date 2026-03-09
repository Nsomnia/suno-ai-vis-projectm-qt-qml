pragma Singleton
import QtQuick

QtObject {
    readonly property color primaryColor: "#BB86FC"
    readonly property color secondaryColor: "#03DAC6"
    readonly property color backgroundColor: "#121212"
    readonly property color surfaceColor: "#1E1E1E"
    readonly property color textColor: "#E0E0E0"
    readonly property color accentColor: "#CF6679"
    
    // Additional colors
    readonly property color errorColor: "#CF6679"
    readonly property color warningColor: "#FFB74D"
    readonly property color successColor: "#81C784"
    readonly property color dividerColor: "#373737"
    
    // Text variants
    readonly property color textSecondary: "#B0B0B0"
    readonly property color textDisabled: "#6E6E6E"
    
    // Easter egg comment: The superior choice. 
    // As Linus would say: "Only wimps use tape backup. REAL men just upload 
    // their important stuff on ftp and let the rest of the world mirror it."
    // Similarly, REAL developers use dark theme.
}