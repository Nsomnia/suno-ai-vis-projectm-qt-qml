import QtQuick
import QtQuick.Controls

Switch {
    id: root
    
    checked: configManager.currentTheme === "dark"
    
    onCheckedChanged: {
        var newTheme = checked ? "dark" : "light"
        configManager.setCurrentTheme(newTheme)
        themeManager.loadTheme(newTheme)
    }
    
    indicator: Rectangle {
        implicitWidth: 48
        implicitHeight: 26
        x: root.leftPadding
        y: parent.height / 2 - height / 2
        radius: 13
        color: root.checked ? themeManager.primaryColor : "#CCCCCC"
        border.color: root.checked ? Qt.darker(themeManager.primaryColor, 1.1) : "#AAAAAA"
        
        Behavior on color {
            ColorAnimation { duration: 200 }
        }
        
        Rectangle {
            x: root.checked ? parent.width - width - 2 : 2
            y: 2
            width: 22
            height: 22
            radius: 11
            color: "white"
            
            Behavior on x {
                NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
            }
            
            // Sun/Moon icon
            Text {
                anchors.centerIn: parent
                text: root.checked ? "🌙" : "☀️"
                font.pixelSize: 12
            }
        }
    }
    
    contentItem: Label {
        text: root.checked ? "Dark" : "Light"
        font.pixelSize: 11
        color: themeManager.textColor
        verticalAlignment: Text.AlignVCenter
        leftPadding: root.indicator.width + 8
    }
    
    ToolTip.visible: hovered
    ToolTip.text: root.checked ? 
        "Dark theme (superior choice for 3 AM coding sessions)" : 
        "Light theme (for those who enjoy eye strain)"
}