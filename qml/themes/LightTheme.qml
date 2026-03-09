pragma Singleton
import QtQuick

QtObject {
    readonly property color primaryColor: "#2196F3"
    readonly property color secondaryColor: "#03A9F4"
    readonly property color backgroundColor: "#FAFAFA"
    readonly property color surfaceColor: "#FFFFFF"
    readonly property color textColor: "#212121"
    readonly property color accentColor: "#FF5722"
    
    // Additional colors
    readonly property color errorColor: "#F44336"
    readonly property color warningColor: "#FFC107"
    readonly property color successColor: "#4CAF50"
    readonly property color dividerColor: "#E0E0E0"
    
    // Text variants
    readonly property color textSecondary: "#757575"
    readonly property color textDisabled: "#9E9E9E"
    
    // Easter egg comment: Light theme, for those who code with the curtains open
    // and have never experienced the joy of a terminal at 3 AM
}