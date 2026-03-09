import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

Window {
    id: sunoWindow
    
    title: "Suno Library - Generate & Browse"
    width: 800
    height: 600
    minimumWidth: 600
    minimumHeight: 400
    
    color: themeManager.backgroundColor
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        
        // Header with tabs
        TabBar {
            id: sunoTabBar
            Layout.fillWidth: true
            
            TabButton { text: "My Tracks" }
            TabButton { text: "Generate" }
        }
        
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: sunoTabBar.currentIndex
            
            // My Tracks tab
            ColumnLayout {
                spacing: 0
                
                // Search bar
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 56
                    color: themeManager.surfaceColor
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 12
                        
                        TextField {
                            Layout.fillWidth: true
                            placeholderText: "Search tracks..."
                        }
                        
                        Button {
                            text: "🔄 Refresh"
                            onClicked: {
                                if (appController.sunoClient.isAuthenticated) {
                                    appController.sunoClient.fetchMyTracks()
                                } else {
                                    authDialog.open()
                                }
                            }
                        }
                    }
                }
                
                // Track list
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    
                    model: 10 // Would be actual track model
                    
                    delegate: ItemDelegate {
                        width: parent.width
                        height: 72
                        
                        contentItem: RowLayout {
                            spacing: 12
                            
                            // Album art placeholder
                            Rectangle {
                                width: 56
                                height: 56
                                radius: 4
                                color: Qt.darker(themeManager.surfaceColor, 1.2)
                                
                                Label {
                                    anchors.centerIn: parent
                                    text: "🎵"
                                    font.pixelSize: 24
                                }
                            }
                            
                            // Track info
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2
                                
                                Label {
                                    text: "Track Title " + (index + 1)
                                    font.pixelSize: 14
                                    font.weight: Font.Medium
                                    color: themeManager.textColor
                                }
                                
                                Label {
                                    text: "Generated on " + Qt.formatDate(new Date(), "MMM d, yyyy")
                                    font.pixelSize: 12
                                    color: Qt.darker(themeManager.textColor, 1.3)
                                }
                                
                                Label {
                                    text: "3:00"
                                    font.pixelSize: 11
                                    color: Qt.darker(themeManager.textColor, 1.5)
                                }
                            }
                            
                            // Actions
                            RowLayout {
                                spacing: 4
                                
                                ToolButton {
                                    icon.source: "qrc:/SunoVisualizer/resources/icons/play.svg"
                                    icon.color: themeManager.textColor
                                    onClicked: {
                                        console.log("Play track", index)
                                    }
                                }
                                
                                ToolButton {
                                    icon.source: "qrc:/SunoVisualizer/resources/icons/download.svg"
                                    icon.color: themeManager.textColor
                                    onClicked: {
                                        console.log("Download track", index)
                                    }
                                }
                                
                                ToolButton {
                                    icon.source: "qrc:/SunoVisualizer/resources/icons/import.svg"
                                    icon.color: themeManager.textColor
                                    ToolTip.visible: hovered
                                    ToolTip.text: "Import to project"
                                    onClicked: {
                                        console.log("Import track", index)
                                    }
                                }
                            }
                        }
                        
                        onClicked: {
                            // Select track
                        }
                    }
                }
                
                // Status bar
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 32
                    color: Qt.darker(themeManager.surfaceColor, 1.05)
                    
                    Label {
                        anchors.centerIn: parent
                        text: appController.sunoClient.isAuthenticated ? 
                              "Connected to Suno" : "Not authenticated"
                        font.pixelSize: 11
                        color: appController.sunoClient.isAuthenticated ? 
                               themeManager.secondaryColor : "#FF8888"
                    }
                }
            }
            
            // Generate tab
            ScrollView {
                clip: true
                
                ColumnLayout {
                    width: parent.width
                    spacing: 16
                    
                    GroupBox {
                        title: "Generate New Track"
                        Layout.fillWidth: true
                        Layout.margins: 16
                        
                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 12
                            
                            Label {
                                text: "Prompt / Description"
                                font.weight: Font.Medium
                            }
                            
                            TextArea {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 100
                                placeholderText: "Describe the song you want to generate...\n\nExample: An upbeat electronic track with synth leads and a driving beat"
                                wrapMode: TextArea.Wrap
                            }
                            
                            Label {
                                text: "Lyrics (optional)"
                                font.weight: Font.Medium
                            }
                            
                            TextArea {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 150
                                placeholderText: "Enter custom lyrics or leave empty for instrumental...\n\n[Verse 1]\nYour lyrics here..."
                                wrapMode: TextArea.Wrap
                            }
                            
                            RowLayout {
                                spacing: 16
                                
                                CheckBox {
                                    text: "Instrumental only"
                                }
                                
                                Label { text: "Style:" }
                                ComboBox {
                                    model: ["Pop", "Rock", "Electronic", "Hip Hop", "Jazz", "Classical", "Ambient"]
                                    Layout.preferredWidth: 150
                                }
                            }
                            
                            Button {
                                text: "🎵 Generate Track"
                                Layout.alignment: Qt.AlignRight
                                highlighted: true
                                
                                onClicked: {
                                    console.log("Generate track")
                                    // Would call appController.sunoClient.generateTrack()
                                }
                            }
                        }
                    }
                    
                    GroupBox {
                        title: "Generation Queue"
                        Layout.fillWidth: true
                        Layout.margins: 16
                        
                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 8
                            
                            Label {
                                text: "No tracks currently generating"
                                color: Qt.darker(themeManager.textColor, 1.3)
                                font.italic: true
                            }
                            
                            // Would show progress for ongoing generations
                        }
                    }
                    
                    Item { Layout.fillHeight: true }
                }
            }
        }
    }
    
    // Auth dialog
    Dialog {
        id: authDialog
        title: "Suno Authentication"
        modal: true
        anchors.centerIn: parent
        standardButtons: Dialog.Ok | Dialog.Cancel
        
        ColumnLayout {
            spacing: 16
            
            Label {
                text: "Enter your Suno session token to access your tracks."
                wrapMode: Text.WordWrap
            }
            
            Label {
                text: "⚠️ This uses unofficial API endpoints"
                color: "#FFA500"
                font.pixelSize: 11
            }
            
            TextField {
                id: tokenField
                Layout.fillWidth: true
                Layout.preferredWidth: 300
                placeholderText: "Session token from browser cookies"
                echoMode: TextInput.Password
            }
        }
        
        onAccepted: {
            appController.sunoClient.authenticate(tokenField.text)
        }
    }
}