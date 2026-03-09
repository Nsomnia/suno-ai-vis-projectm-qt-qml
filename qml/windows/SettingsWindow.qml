import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

Window {
    id: settingsWindow
    
    title: "Settings - Suno Visualizer"
    width: 600
    height: 500
    minimumWidth: 500
    minimumHeight: 400
    
    color: themeManager.backgroundColor
    
    flags: Qt.Dialog | Qt.WindowCloseButtonHint | Qt.WindowTitleHint
    modality: Qt.ApplicationModal
    
    onClosing: function(close) {
        close.accepted = true
        settingsWindow.hide()
    }
    
    Shortcut {
        sequence: "Escape"
        onActivated: settingsWindow.hide()
    }
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 0
        spacing: 0
        
        // Tab bar
        TabBar {
            id: tabBar
            Layout.fillWidth: true
            
            TabButton { text: "General" }
            TabButton { text: "Video" }
            TabButton { text: "Audio" }
            TabButton { text: "Visualizer" }
            TabButton { text: "Network" }
            TabButton { text: "Git" }
        }
        
        // Tab content
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabBar.currentIndex
            
            // General Settings
            ScrollView {
                clip: true
                
                ColumnLayout {
                    width: parent.width
                    spacing: 16
                    
                    GroupBox {
                        title: "Appearance"
                        Layout.fillWidth: true
                        Layout.margins: 16
                        
                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 12
                            
                            RowLayout {
                                Label { 
                                    text: "Theme:" 
                                    Layout.preferredWidth: 120
                                }
                                ComboBox {
                                    model: ["Dark", "Light"]
                                    currentIndex: configManager.currentTheme === "dark" ? 0 : 1
                                    onCurrentIndexChanged: {
                                        var theme = currentIndex === 0 ? "dark" : "light"
                                        configManager.setCurrentTheme(theme)
                                        themeManager.loadTheme(theme)
                                    }
                                }
                            }
                        }
                    }
                    
                    GroupBox {
                        title: "Auto-save"
                        Layout.fillWidth: true
                        Layout.margins: 16
                        
                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 12
                            
                            CheckBox {
                                text: "Enable auto-save"
                                checked: configManager.autoSaveEnabled
                                onCheckedChanged: configManager.setAutoSaveEnabled(checked)
                            }
                            
                            RowLayout {
                                Label { 
                                    text: "Interval (seconds):" 
                                    Layout.preferredWidth: 120
                                }
                                SpinBox {
                                    from: 30
                                    to: 3600
                                    value: 300
                                    editable: true
                                }
                            }
                        }
                    }
                    
                    Item { Layout.fillHeight: true }
                }
            }
            
            // Video Settings
            ScrollView {
                clip: true
                
                ColumnLayout {
                    width: parent.width
                    spacing: 16
                    
                    GroupBox {
                        title: "Video Export"
                        Layout.fillWidth: true
                        Layout.margins: 16
                        
                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 12
                            
                            RowLayout {
                                Label { 
                                    text: "Codec:" 
                                    Layout.preferredWidth: 120
                                }
                                ComboBox {
                                    model: ["H.264 (libx264)", "H.265 (libx265)", "VP9", "ProRes"]
                                    Layout.fillWidth: true
                                }
                            }
                            
                            RowLayout {
                                Label { 
                                    text: "Bitrate (Mbps):" 
                                    Layout.preferredWidth: 120
                                }
                                SpinBox {
                                    from: 1
                                    to: 100
                                    value: 8
                                    editable: true
                                }
                            }
                            
                            RowLayout {
                                Label { 
                                    text: "FPS:" 
                                    Layout.preferredWidth: 120
                                }
                                ComboBox {
                                    model: ["24", "30", "60", "120"]
                                    currentIndex: 2
                                }
                            }
                            
                            RowLayout {
                                Label { 
                                    text: "Resolution:" 
                                    Layout.preferredWidth: 120
                                }
                                ComboBox {
                                    model: ["1280x720", "1920x1080", "2560x1440", "3840x2160"]
                                    currentIndex: 1
                                }
                            }
                        }
                    }
                    
                    Item { Layout.fillHeight: true }
                }
            }
            
            // Audio Settings
            ScrollView {
                clip: true
                
                ColumnLayout {
                    width: parent.width
                    spacing: 16
                    
                    GroupBox {
                        title: "Audio Processing"
                        Layout.fillWidth: true
                        Layout.margins: 16
                        
                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 12
                            
                            RowLayout {
                                Label { 
                                    text: "Sample Rate:" 
                                    Layout.preferredWidth: 120
                                }
                                ComboBox {
                                    model: ["44100 Hz", "48000 Hz", "96000 Hz"]
                                    currentIndex: 1
                                }
                            }
                            
                            RowLayout {
                                Label { 
                                    text: "Buffer Size:" 
                                    Layout.preferredWidth: 120
                                }
                                ComboBox {
                                    model: ["512", "1024", "2048", "4096"]
                                    currentIndex: 2
                                }
                            }
                        }
                    }
                    
                    GroupBox {
                        title: "Stem Separation"
                        Layout.fillWidth: true
                        Layout.margins: 16
                        
                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 12
                            
                            RowLayout {
                                Label { 
                                    text: "Model:" 
                                    Layout.preferredWidth: 120
                                }
                                ComboBox {
                                    model: ["htdemucs", "htdemucs_ft", "mdx_extra"]
                                }
                            }
                            
                            Label {
                                text: "Note: Stem separation requires Demucs to be installed.\npip install demucs"
                                font.pixelSize: 11
                                color: Qt.darker(themeManager.textColor, 1.3)
                            }
                        }
                    }
                    
                    Item { Layout.fillHeight: true }
                }
            }
            
            // Visualizer Settings
            ScrollView {
                clip: true
                
                ColumnLayout {
                    width: parent.width
                    spacing: 16
                    
                    GroupBox {
                        title: "projectM Settings"
                        Layout.fillWidth: true
                        Layout.margins: 16
                        
                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 12
                            
                            RowLayout {
                                Label { 
                                    text: "Mesh Size X:" 
                                    Layout.preferredWidth: 120
                                }
                                SpinBox {
                                    from: 32
                                    to: 512
                                    value: 220
                                }
                            }
                            
                            RowLayout {
                                Label { 
                                    text: "Mesh Size Y:" 
                                    Layout.preferredWidth: 120
                                }
                                SpinBox {
                                    from: 32
                                    to: 512
                                    value: 125
                                }
                            }
                            
                            RowLayout {
                                Label { 
                                    text: "Beat Sensitivity:" 
                                    Layout.preferredWidth: 120
                                }
                                Slider {
                                    from: 0.0
                                    to: 3.0
                                    value: 1.0
                                    Layout.fillWidth: true
                                }
                                Label {
                                    text: "1.0"
                                    font.pixelSize: 11
                                }
                            }
                            
                            RowLayout {
                                Label { 
                                    text: "Preset Duration (s):" 
                                    Layout.preferredWidth: 120
                                }
                                SpinBox {
                                    from: 5
                                    to: 300
                                    value: 30
                                }
                            }
                            
                            CheckBox {
                                text: "Aspect correction"
                                checked: true
                            }
                            
                            CheckBox {
                                text: "Hard cuts enabled"
                                checked: true
                            }
                        }
                    }
                    
                    Item { Layout.fillHeight: true }
                }
            }
            
            // Network Settings
            ScrollView {
                clip: true
                
                ColumnLayout {
                    width: parent.width
                    spacing: 16
                    
                    GroupBox {
                        title: "Suno API"
                        Layout.fillWidth: true
                        Layout.margins: 16
                        
                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 12
                            
                            Label {
                                text: "⚠️ Unofficial API - Use responsibly!"
                                color: "#FFA500"
                                font.weight: Font.Medium
                            }
                            
                            RowLayout {
                                Label { 
                                    text: "Session Token:" 
                                    Layout.preferredWidth: 120
                                }
                                TextField {
                                    Layout.fillWidth: true
                                    echoMode: TextInput.Password
                                    placeholderText: "Paste your session token here"
                                }
                            }
                        }
                    }
                    
                    GroupBox {
                        title: "GitHub (for Gist sharing)"
                        Layout.fillWidth: true
                        Layout.margins: 16
                        
                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 12
                            
                            RowLayout {
                                Label { 
                                    text: "Personal Token:" 
                                    Layout.preferredWidth: 120
                                }
                                TextField {
                                    Layout.fillWidth: true
                                    echoMode: TextInput.Password
                                    placeholderText: "ghp_xxxxxxxxxxxx"
                                }
                            }
                            
                            Label {
                                text: "Generate a token at github.com/settings/tokens\nRequired scope: gist"
                                font.pixelSize: 11
                                color: Qt.darker(themeManager.textColor, 1.3)
                            }
                        }
                    }
                    
                    Item { Layout.fillHeight: true }
                }
            }
            
            // Git Settings
            ScrollView {
                clip: true
                
                ColumnLayout {
                    width: parent.width
                    spacing: 16
                    
                    GroupBox {
                        title: "Git Identity"
                        Layout.fillWidth: true
                        Layout.margins: 16
                        
                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 12
                            
                            Label {
                                text: "\"Talk is cheap. Show me the code.\" - Linus Torvalds"
                                font.italic: true
                                color: Qt.darker(themeManager.textColor, 1.3)
                            }
                            
                            RowLayout {
                                Label { 
                                    text: "User Name:" 
                                    Layout.preferredWidth: 120
                                }
                                TextField {
                                    Layout.fillWidth: true
                                    text: configManager.gitUserName
                                    onTextChanged: configManager.setGitUserName(text)
                                }
                            }
                            
                            RowLayout {
                                Label { 
                                    text: "Email:" 
                                    Layout.preferredWidth: 120
                                }
                                TextField {
                                    Layout.fillWidth: true
                                    text: configManager.gitUserEmail
                                    onTextChanged: configManager.setGitUserEmail(text)
                                }
                            }
                            
                            CheckBox {
                                text: "Auto-commit changes"
                                checked: configManager.gitAutoCommit
                                onCheckedChanged: configManager.setGitAutoCommit(checked)
                            }
                        }
                    }
                    
                    Item { Layout.fillHeight: true }
                }
            }
        }
        
        // Bottom buttons
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 56
            color: Qt.darker(themeManager.surfaceColor, 1.05)
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 12
                
                Item { Layout.fillWidth: true }
                
                Button {
                    text: "Reset to Defaults"
                    flat: true
                    onClicked: {
                        // Reset settings
                        console.log("Reset settings to defaults")
                    }
                }
                
                Button {
                    text: "Cancel"
                    onClicked: settingsWindow.close()
                }
                
                Button {
                    text: "Save"
                    highlighted: true
                    onClicked: {
                        configManager.save()
                        settingsWindow.close()
                    }
                }
            }
        }
    }
}