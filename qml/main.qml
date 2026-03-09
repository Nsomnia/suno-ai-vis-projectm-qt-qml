import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import SunoVisualizer

ApplicationWindow {
    id: mainWindow
    
    width: 1600
    height: 900
    minimumWidth: 1200
    minimumHeight: 700
    visible: true
    title: qsTr("Suno Visualizer - The Future is Visual")
    
    color: themeManager.backgroundColor
    
    // Window state
    property bool leftDrawerOpen: true
    property bool rightDrawerOpen: false
    property bool isPlaying: appController.isPlaying
    property bool isRecording: appController.isRecording
    
    // Keyboard shortcuts
    Shortcut {
        sequence: "Space"
        onActivated: isPlaying ? appController.pause() : appController.play()
    }
    
    Shortcut {
        sequence: "Ctrl+S"
        onActivated: appController.saveProject()
    }
    
    Shortcut {
        sequence: "Ctrl+O"
        onActivated: fileDialog.open()
    }
    
    Shortcut {
        sequence: "F11"
        onActivated: {
            if (mainWindow.visibility === Window.FullScreen) {
                mainWindow.showNormal()
            } else {
                mainWindow.showFullScreen()
            }
        }
    }
    
    // Main layout
    RowLayout {
        anchors.fill: parent
        spacing: 0
        
        // Left Accordion Drawer
        AccordionDrawer {
            id: leftDrawer
            Layout.fillHeight: true
            Layout.preferredWidth: leftDrawerOpen ? 280 : 48
            
            Behavior on Layout.preferredWidth {
                NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
            }
            
            isOpen: leftDrawerOpen
            side: "left"
            
            onToggleRequested: leftDrawerOpen = !leftDrawerOpen
            
            sections: [
                {
                    "title": "Project",
                    "icon": "qrc:/icons/folder.svg",
                    "content": "ProjectSection"
                },
                {
                    "title": "Presets",
                    "icon": "qrc:/icons/sparkles.svg",
                    "content": "PresetsSection"
                },
                {
                    "title": "Audio",
                    "icon": "qrc:/icons/music.svg",
                    "content": "AudioSection"
                },
                {
                    "title": "Stems",
                    "icon": "qrc:/icons/waveform.svg",
                    "content": "StemsSection"
                }
            ]
        }
        
        // Center Content Area
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0
            
            // Top toolbar
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 56
                color: themeManager.surfaceColor
                
                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    spacing: 12
                    
                    // Menu button
                    ToolButton {
                        icon.source: "qrc:/icons/menu.svg"
                        icon.color: themeManager.textColor
                        onClicked: leftDrawerOpen = !leftDrawerOpen
                        
                        ToolTip.visible: hovered
                        ToolTip.text: "Toggle sidebar (Arch users: you know the keybind)"
                    }
                    
                    // Track info
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        
                        Label {
                            text: appController.currentTrackTitle || "No track loaded"
                            font.pixelSize: 16
                            font.weight: Font.Medium
                            color: themeManager.textColor
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                        
                        Label {
                            text: appController.currentTrackArtist || "Load a track to begin"
                            font.pixelSize: 12
                            color: Qt.darker(themeManager.textColor, 1.3)
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }
                    
                    // Theme switch
                    ThemeSwitch {
                        id: themeSwitch
                    }
                    
                    // Settings button
                    ToolButton {
                        icon.source: "qrc:/icons/settings.svg"
                        icon.color: themeManager.textColor
                        onClicked: settingsWindow.show()
                        
                        ToolTip.visible: hovered
                        ToolTip.text: "Settings"
                    }
                    
                    // Right drawer toggle
                    ToolButton {
                        icon.source: "qrc:/icons/sidebar-right.svg"
                        icon.color: themeManager.textColor
                        onClicked: rightDrawerOpen = !rightDrawerOpen
                        
                        ToolTip.visible: hovered
                        ToolTip.text: "Shader Editor / Lyrics"
                    }
                }
            }
            
            // Visualizer Canvas (main focus area)
            VisualizerCanvas {
                id: visualizerCanvas
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
            
            // Timeline Scrubber
            TimelineScrubber {
                id: timelineScrubber
                Layout.fillWidth: true
                Layout.preferredHeight: 120
                
                duration: appController.audioEngine ? appController.audioEngine.duration : 0
                position: appController.audioEngine ? appController.audioEngine.position : 0
                
                onSeekRequested: function(positionMs) {
                    appController.seek(positionMs)
                }
            }
            
            // Playback controls bar
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 72
                color: themeManager.surfaceColor
                
                RowLayout {
                    anchors.centerIn: parent
                    spacing: 24
                    
                    // Previous
                    RoundButton {
                        icon.source: "qrc:/icons/skip-back.svg"
                        icon.color: themeManager.textColor
                        icon.width: 24
                        icon.height: 24
                        flat: true
                        onClicked: appController.visualizer.previousPreset()
                    }
                    
                    // Play/Pause
                    RoundButton {
                        id: playButton
                        implicitWidth: 56
                        implicitHeight: 56
                        
                        icon.source: isPlaying ? "qrc:/icons/pause.svg" : "qrc:/icons/play.svg"
                        icon.color: themeManager.backgroundColor
                        icon.width: 28
                        icon.height: 28
                        
                        background: Rectangle {
                            color: themeManager.accentColor
                            radius: width / 2
                        }
                        
                        onClicked: isPlaying ? appController.pause() : appController.play()
                        
                        ToolTip.visible: hovered
                        ToolTip.text: isPlaying ? "Pause (Space)" : "Play (Space)"
                    }
                    
                    // Next
                    RoundButton {
                        icon.source: "qrc:/icons/skip-forward.svg"
                        icon.color: themeManager.textColor
                        icon.width: 24
                        icon.height: 24
                        flat: true
                        onClicked: appController.visualizer.nextPreset()
                    }
                    
                    // Separator
                    Rectangle {
                        width: 1
                        height: 32
                        color: Qt.darker(themeManager.surfaceColor, 1.2)
                    }
                    
                    // Record button
                    RoundButton {
                        icon.source: "qrc:/icons/record.svg"
                        icon.color: isRecording ? "#FF4444" : themeManager.textColor
                        icon.width: 24
                        icon.height: 24
                        flat: true
                        
                        onClicked: {
                            if (isRecording) {
                                appController.stopRecording()
                            } else {
                                recordDialog.open()
                            }
                        }
                        
                        ToolTip.visible: hovered
                        ToolTip.text: isRecording ? "Stop Recording" : "Start Recording"
                        
                        // Pulsing animation when recording
                        SequentialAnimation on opacity {
                            running: isRecording
                            loops: Animation.Infinite
                            NumberAnimation { to: 0.5; duration: 500 }
                            NumberAnimation { to: 1.0; duration: 500 }
                        }
                    }
                    
                    // Volume control
                    RowLayout {
                        spacing: 8
                        
                        Image {
                            source: "qrc:/icons/volume.svg"
                            sourceSize: Qt.size(20, 20)
                            
                            // Simple color overlay would go here
                        }
                        
                        Slider {
                            id: volumeSlider
                            implicitWidth: 100
                            from: 0
                            to: 1
                            value: appController.audioEngine ? appController.audioEngine.volume : 1.0
                            
                            onValueChanged: {
                                if (appController.audioEngine) {
                                    appController.audioEngine.setVolume(value)
                                }
                            }
                        }
                    }
                }
            }
        }
        
        // Right Accordion Drawer (Shader IDE / Lyrics)
        AccordionDrawer {
            id: rightDrawer
            Layout.fillHeight: true
            Layout.preferredWidth: rightDrawerOpen ? 360 : 48
            
            Behavior on Layout.preferredWidth {
                NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
            }
            
            isOpen: rightDrawerOpen
            side: "right"
            
            onToggleRequested: rightDrawerOpen = !rightDrawerOpen
            
            sections: [
                {
                    "title": "Shader Editor",
                    "icon": "qrc:/icons/code.svg",
                    "content": "ShaderEditorSection"
                },
                {
                    "title": "Lyrics",
                    "icon": "qrc:/icons/text.svg",
                    "content": "LyricsSection"
                },
                {
                    "title": "Community",
                    "icon": "qrc:/icons/users.svg",
                    "content": "CommunitySection"
                }
            ]
        }
    }
    
    // Status bar
    footer: Rectangle {
        height: 24
        color: Qt.darker(themeManager.surfaceColor, 1.1)
        
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            
            Label {
                id: statusLabel
                text: "Ready"
                font.pixelSize: 11
                color: Qt.darker(themeManager.textColor, 1.3)
            }
            
            Item { Layout.fillWidth: true }
            
            // Easter egg - click for wisdom
            Label {
                text: "💡"
                font.pixelSize: 11
                
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        statusLabel.text = appController.getSystemWisdom()
                    }
                    cursorShape: Qt.PointingHandCursor
                }
                
                ToolTip.visible: hovered
                ToolTip.text: "Click for wisdom from the masters"
            }
            
            Label {
                text: isRecording ? "● REC" : ""
                font.pixelSize: 11
                font.weight: Font.Bold
                color: "#FF4444"
                visible: isRecording
            }
            
            Label {
                text: "v0.0.1"
                font.pixelSize: 11
                color: Qt.darker(themeManager.textColor, 1.5)
            }
        }
    }
    
    // Settings Window
    SettingsWindow {
        id: settingsWindow
    }
    
    // Suno Queue Window
    SunoQueueWindow {
        id: sunoQueueWindow
    }
    
    // File dialog for opening audio files
    // Note: In Qt6, use Qt.labs.platform or native dialogs
    Loader {
        id: fileDialogLoader
        active: false
        sourceComponent: Component {
            // Placeholder - would use native file dialog
            Item {}
        }
    }
    
    // Record output dialog (simplified)
    Dialog {
        id: recordDialog
        title: "Start Recording"
        standardButtons: Dialog.Ok | Dialog.Cancel
        modal: true
        anchors.centerIn: parent
        
        ColumnLayout {
            spacing: 16
            
            Label {
                text: "Output filename:"
                color: themeManager.textColor
            }
            
            TextField {
                id: recordFilename
                text: "output_" + Qt.formatDateTime(new Date(), "yyyyMMdd_hhmmss") + ".mp4"
                Layout.fillWidth: true
            }
        }
        
        onAccepted: {
            var outputPath = configManager.getValue("lastExportDir", "") + "/" + recordFilename.text
            appController.startRecording(outputPath)
        }
    }
    
    // Connections for status messages
    Connections {
        target: appController
        
        function onStatusMessage(message) {
            statusLabel.text = message
        }
        
        function onErrorOccurred(error) {
            statusLabel.text = "Error: " + error
            errorDialog.text = error
            errorDialog.open()
        }
    }
    
    // Error dialog
    Dialog {
        id: errorDialog
        title: "Error"
        standardButtons: Dialog.Ok
        modal: true
        anchors.centerIn: parent
        
        property alias text: errorLabel.text
        
        Label {
            id: errorLabel
            color: themeManager.textColor
            wrapMode: Text.WordWrap
        }
    }
    
    // Startup message
    Component.onCompleted: {
        console.log("Suno Visualizer started successfully")
        console.log(appController.getSystemWisdom())
    }
}