import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    
    property real duration: 180000 // milliseconds
    property real position: 0
    
    signal seekRequested(real positionMs)
    
    color: themeManager.surfaceColor
    
    // Top border
    Rectangle {
        anchors.top: parent.top
        width: parent.width
        height: 1
        color: Qt.darker(themeManager.surfaceColor, 1.2)
    }
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 4
        
        // Time display row
        RowLayout {
            Layout.fillWidth: true
            spacing: 16
            
            Label {
                text: formatTime(position)
                font.pixelSize: 12
                font.family: "monospace"
                color: themeManager.textColor
            }
            
            Item { Layout.fillWidth: true }
            
            Label {
                text: formatTime(duration)
                font.pixelSize: 12
                font.family: "monospace"
                color: Qt.darker(themeManager.textColor, 1.3)
            }
        }
        
        // Timeline area
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Qt.darker(themeManager.surfaceColor, 1.15)
            radius: 4
            clip: true
            
            // Timeline tracks
            Column {
                anchors.fill: parent
                anchors.margins: 4
                spacing: 2
                
                // Preset track
                TimelineTrack {
                    width: parent.width
                    height: 24
                    trackName: "Presets"
                    trackColor: themeManager.primaryColor
                    duration: root.duration
                    position: root.position
                }
                
                // Lyrics track
                TimelineTrack {
                    width: parent.width
                    height: 24
                    trackName: "Lyrics"
                    trackColor: themeManager.secondaryColor
                    duration: root.duration
                    position: root.position
                }
                
                // Markers track
                TimelineTrack {
                    width: parent.width
                    height: 24
                    trackName: "Markers"
                    trackColor: themeManager.accentColor
                    duration: root.duration
                    position: root.position
                }
            }
            
            // Playhead
            Rectangle {
                id: playhead
                x: (root.position / root.duration) * parent.width - 1
                y: 0
                width: 2
                height: parent.height
                color: themeManager.accentColor
                
                Behavior on x {
                    enabled: !scrubArea.pressed
                    NumberAnimation { duration: 50 }
                }
                
                // Playhead handle
                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    width: 12
                    height: 12
                    radius: 6
                    color: themeManager.accentColor
                    
                    Rectangle {
                        anchors.centerIn: parent
                        width: 6
                        height: 6
                        radius: 3
                        color: "white"
                    }
                }
            }
            
            // Scrub interaction area
            MouseArea {
                id: scrubArea
                anchors.fill: parent
                
                onPressed: function(mouse) {
                    seek(mouse.x)
                }
                
                onPositionChanged: function(mouse) {
                    if (pressed) {
                        seek(mouse.x)
                    }
                }
                
                function seek(x) {
                    var pos = Math.max(0, Math.min(1, x / width)) * root.duration
                    root.seekRequested(pos)
                }
            }
        }
        
        // Zoom controls
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            
            Label {
                text: "Zoom:"
                font.pixelSize: 11
                color: Qt.darker(themeManager.textColor, 1.3)
            }
            
            Slider {
                id: zoomSlider
                Layout.preferredWidth: 100
                from: 1
                to: 10
                value: 1
            }
            
            Item { Layout.fillWidth: true }
            
            // Timeline tools
            ToolButton {
                icon.source: "qrc:/icons/git-commit.svg"
                icon.color: themeManager.textColor
                icon.width: 16
                icon.height: 16
                
                ToolTip.visible: hovered
                ToolTip.text: "View Git history (because every frame matters)"
            }
            
            ToolButton {
                icon.source: "qrc:/icons/plus.svg"
                icon.color: themeManager.textColor
                icon.width: 16
                icon.height: 16
                
                ToolTip.visible: hovered
                ToolTip.text: "Add marker"
            }
        }
    }
    
    // Timeline track component
    component TimelineTrack: Rectangle {
        property string trackName: "Track"
        property color trackColor: themeManager.primaryColor
        property real duration: 180000
        property real position: 0
        
        color: Qt.darker(themeManager.surfaceColor, 1.25)
        radius: 2
        
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 4
            spacing: 4
            
            // Track label
            Label {
                text: trackName
                font.pixelSize: 10
                color: Qt.darker(themeManager.textColor, 1.2)
                Layout.preferredWidth: 50
            }
            
            // Track content area
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.margins: 2
                color: "transparent"
                
                // Sample timeline entries (would be populated from model)
                Repeater {
                    model: 3
                    
                    Rectangle {
                        x: (index * 0.25 + 0.1) * parent.width
                        y: 2
                        width: parent.width * 0.15
                        height: parent.height - 4
                        radius: 2
                        color: Qt.rgba(trackColor.r, trackColor.g, trackColor.b, 0.6)
                        border.color: trackColor
                        border.width: 1
                        
                        Label {
                            anchors.centerIn: parent
                            text: "Entry " + (index + 1)
                            font.pixelSize: 8
                            color: "white"
                            visible: parent.width > 40
                        }
                    }
                }
            }
        }
    }
    
    // Time formatting helper
    function formatTime(ms) {
        var totalSeconds = Math.floor(ms / 1000)
        var minutes = Math.floor(totalSeconds / 60)
        var seconds = totalSeconds % 60
        var millis = Math.floor((ms % 1000) / 10)
        
        return pad(minutes, 2) + ":" + pad(seconds, 2) + "." + pad(millis, 2)
    }
    
    function pad(num, size) {
        var s = "0" + num
        return s.substr(s.length - size)
    }
}