import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    
    property bool isOpen: true
    property string side: "left" // "left" or "right"
    property var sections: []
    
    signal toggleRequested()
    
    color: themeManager.surfaceColor
    
    // Subtle border
    Rectangle {
        width: 1
        height: parent.height
        color: Qt.darker(themeManager.surfaceColor, 1.2)
        anchors.right: side === "left" ? parent.right : undefined
        anchors.left: side === "right" ? parent.left : undefined
    }
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        
        // Toggle button at top
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            color: "transparent"
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                layoutDirection: side === "left" ? Qt.LeftToRight : Qt.RightToLeft
                
                ToolButton {
                    icon.source: isOpen 
                        ? (side === "left" ? "qrc:/icons/chevron-left.svg" : "qrc:/icons/chevron-right.svg")
                        : (side === "left" ? "qrc:/icons/chevron-right.svg" : "qrc:/icons/chevron-left.svg")
                    icon.color: themeManager.textColor
                    onClicked: root.toggleRequested()
                }
                
                Label {
                    visible: isOpen
                    text: side === "left" ? "Tools" : "Editor"
                    font.pixelSize: 14
                    font.weight: Font.Medium
                    color: themeManager.textColor
                    Layout.fillWidth: true
                }
            }
        }
        
        // Divider
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Qt.darker(themeManager.surfaceColor, 1.2)
        }
        
        // Accordion sections
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            
            ColumnLayout {
                width: parent.width
                spacing: 0
                
                Repeater {
                    model: sections
                    
                    delegate: AccordionSection {
                        Layout.fillWidth: true
                        title: modelData.title
                        iconSource: modelData.icon
                        contentType: modelData.content
                        isCollapsed: !isOpen
                        showLabels: isOpen
                    }
                }
            }
        }
    }
    
    // Accordion Section Component
    component AccordionSection: Rectangle {
        id: section
        
        property string title: "Section"
        property string iconSource: ""
        property string contentType: ""
        property bool isCollapsed: false
        property bool showLabels: true
        property bool expanded: false
        
        implicitHeight: headerRow.height + (expanded && showLabels ? contentLoader.height : 0)
        color: "transparent"
        
        Behavior on implicitHeight {
            NumberAnimation { duration: 150; easing.type: Easing.OutCubic }
        }
        
        ColumnLayout {
            anchors.fill: parent
            spacing: 0
            
            // Section header
            Rectangle {
                id: headerRow
                Layout.fillWidth: true
                Layout.preferredHeight: 44
                color: sectionMouse.containsMouse ? Qt.lighter(themeManager.surfaceColor, 1.1) : "transparent"
                
                MouseArea {
                    id: sectionMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        if (showLabels) {
                            section.expanded = !section.expanded
                        }
                    }
                }
                
                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    spacing: 12
                    
                    Image {
                        source: iconSource
                        sourceSize: Qt.size(20, 20)
                        opacity: 0.8
                        
                        // Color overlay would be applied here
                    }
                    
                    Label {
                        visible: showLabels
                        text: title
                        font.pixelSize: 13
                        font.weight: Font.Medium
                        color: themeManager.textColor
                        Layout.fillWidth: true
                    }
                    
                    Image {
                        visible: showLabels
                        source: expanded ? "qrc:/icons/chevron-up.svg" : "qrc:/icons/chevron-down.svg"
                        sourceSize: Qt.size(16, 16)
                        opacity: 0.6
                    }
                }
            }
            
            // Section content
            Loader {
                id: contentLoader
                Layout.fillWidth: true
                visible: expanded && showLabels
                active: expanded && showLabels
                
                sourceComponent: {
                    switch(contentType) {
                        case "ProjectSection": return projectSectionComponent
                        case "PresetsSection": return presetsSectionComponent
                        case "AudioSection": return audioSectionComponent
                        case "StemsSection": return stemsSectionComponent
                        case "ShaderEditorSection": return shaderEditorComponent
                        case "LyricsSection": return lyricsSectionComponent
                        case "CommunitySection": return communitySectionComponent
                        default: return defaultSectionComponent
                    }
                }
            }
        }
        
        // Divider
        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: Qt.darker(themeManager.surfaceColor, 1.15)
        }
    }
    
    // Section Components
    Component {
        id: defaultSectionComponent
        
        Rectangle {
            implicitHeight: 100
            color: "transparent"
            
            Label {
                anchors.centerIn: parent
                text: "Coming soon..."
                color: Qt.darker(themeManager.textColor, 1.5)
                font.italic: true
            }
        }
    }
    
    Component {
        id: projectSectionComponent
        
        ColumnLayout {
            spacing: 8
            
            Repeater {
                model: [
                    { text: "New Project", icon: "qrc:/icons/file-plus.svg" },
                    { text: "Open Project", icon: "qrc:/icons/folder-open.svg" },
                    { text: "Save Project", icon: "qrc:/icons/save.svg" },
                    { text: "Export Video", icon: "qrc:/icons/video.svg" }
                ]
                
                delegate: Button {
                    Layout.fillWidth: true
                    Layout.leftMargin: 12
                    Layout.rightMargin: 12
                    text: modelData.text
                    flat: true
                    
                    contentItem: RowLayout {
                        spacing: 8
                        Image {
                            source: modelData.icon
                            sourceSize: Qt.size(16, 16)
                        }
                        Label {
                            text: modelData.text
                            color: themeManager.textColor
                            font.pixelSize: 12
                        }
                        Item { Layout.fillWidth: true }
                    }
                }
            }
            
            Item { implicitHeight: 8 }
        }
    }
    
    Component {
        id: presetsSectionComponent
        
        ColumnLayout {
            spacing: 8
            
            // Search field
            TextField {
                Layout.fillWidth: true
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                placeholderText: "Search presets..."
                font.pixelSize: 12
            }
            
            // Preset list
            ListView {
                Layout.fillWidth: true
                Layout.preferredHeight: 200
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                clip: true
                
                model: appController.presetManager ? appController.presetManager.presetCount : 0
                
                delegate: ItemDelegate {
                    width: parent.width
                    height: 36
                    
                    contentItem: RowLayout {
                        Label {
                            text: "Preset " + (index + 1)
                            color: themeManager.textColor
                            font.pixelSize: 12
                        }
                        Item { Layout.fillWidth: true }
                        Image {
                            source: "qrc:/icons/star.svg"
                            sourceSize: Qt.size(14, 14)
                            opacity: 0.5
                        }
                    }
                    
                    onClicked: {
                        appController.visualizer.loadPresetByIndex(index)
                    }
                }
            }
            
            // Random preset button
            Button {
                Layout.fillWidth: true
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                text: "🎲 Random Preset"
                onClicked: appController.visualizer.randomPreset()
            }
            
            Item { implicitHeight: 8 }
        }
    }
    
    Component {
        id: audioSectionComponent
        
        ColumnLayout {
            spacing: 12
            
            Button {
                Layout.fillWidth: true
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                text: "📂 Load Audio File"
                onClicked: {
                    // Would open file dialog
                    console.log("Load audio file clicked")
                }
            }
            
            Button {
                Layout.fillWidth: true
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                text: "🎵 Suno Library"
                onClicked: sunoQueueWindow.show()
            }
            
            // Audio info
            GroupBox {
                Layout.fillWidth: true
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                title: "Audio Info"
                
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4
                    
                    Label {
                        text: "Sample Rate: 48000 Hz"
                        font.pixelSize: 11
                        color: Qt.darker(themeManager.textColor, 1.3)
                    }
                    Label {
                        text: "Channels: Stereo"
                        font.pixelSize: 11
                        color: Qt.darker(themeManager.textColor, 1.3)
                    }
                    Label {
                        text: "Duration: --:--"
                        font.pixelSize: 11
                        color: Qt.darker(themeManager.textColor, 1.3)
                    }
                }
            }
            
            Item { implicitHeight: 8 }
        }
    }
    
    Component {
        id: stemsSectionComponent
        
        ColumnLayout {
            spacing: 12
            
            Label {
                Layout.leftMargin: 12
                text: "AI Stem Separation"
                font.pixelSize: 12
                font.weight: Font.Medium
                color: themeManager.textColor
            }
            
            Button {
                Layout.fillWidth: true
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                text: appController.stemProcessor.isProcessing ? "Processing..." : "🎛️ Extract Stems"
                enabled: !appController.stemProcessor.isProcessing
                
                onClicked: {
                    // Would trigger stem extraction
                    console.log("Extract stems clicked")
                }
            }
            
            ProgressBar {
                Layout.fillWidth: true
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                visible: appController.stemProcessor.isProcessing
                value: appController.stemProcessor.progress / 100
            }
            
            // Stem channels
            Repeater {
                model: ["Vocals", "Bass", "Drums", "Other"]
                
                delegate: RowLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: 12
                    Layout.rightMargin: 12
                    
                    CheckBox {
                        checked: true
                        text: modelData
                    }
                    
                    Item { Layout.fillWidth: true }
                    
                    Slider {
                        implicitWidth: 80
                        from: 0
                        to: 1
                        value: 1
                    }
                }
            }
            
            Item { implicitHeight: 8 }
        }
    }
    
    Component {
        id: shaderEditorComponent
        
        ColumnLayout {
            spacing: 8
            
            Label {
                Layout.leftMargin: 12
                text: "MilkDrop Preset Editor"
                font.pixelSize: 12
                font.weight: Font.Medium
                color: themeManager.textColor
            }
            
            // Mini code editor
            ScrollView {
                Layout.fillWidth: true
                Layout.preferredHeight: 300
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                
                TextArea {
                    id: shaderEditor
                    font.family: "monospace"
                    font.pixelSize: 11
                    color: themeManager.textColor
                    background: Rectangle {
                        color: Qt.darker(themeManager.surfaceColor, 1.2)
                        radius: 4
                    }
                    
                    text: "// MilkDrop Preset Editor\n// Edit your shader code here\n\nper_frame_1=wave_r = 0.5 + 0.5*sin(time);\nper_frame_2=wave_g = 0.5 + 0.5*cos(time*1.3);\nper_frame_3=wave_b = 0.5 + 0.5*sin(time*0.7);"
                    
                    wrapMode: TextArea.NoWrap
                }
            }
            
            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                
                Button {
                    text: "Apply"
                    onClicked: {
                        console.log("Apply shader changes")
                    }
                }
                
                Button {
                    text: "Save"
                    onClicked: {
                        console.log("Save preset")
                    }
                }
                
                Item { Layout.fillWidth: true }
                
                Button {
                    text: "📤 Share"
                    onClicked: {
                        console.log("Share to Gist")
                    }
                }
            }
            
            Item { implicitHeight: 8 }
        }
    }
    
    Component {
        id: lyricsSectionComponent
        
        ColumnLayout {
            spacing: 8
            
            Label {
                Layout.leftMargin: 12
                text: "Lyric Layer Editor"
                font.pixelSize: 12
                font.weight: Font.Medium
                color: themeManager.textColor
            }
            
            // Lyrics text area
            ScrollView {
                Layout.fillWidth: true
                Layout.preferredHeight: 200
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                
                TextArea {
                    id: lyricsEditor
                    font.pixelSize: 12
                    color: themeManager.textColor
                    background: Rectangle {
                        color: Qt.darker(themeManager.surfaceColor, 1.2)
                        radius: 4
                    }
                    
                    placeholderText: "Paste or fetch lyrics here...\n\n[00:00.00] First line\n[00:05.00] Second line"
                    
                    wrapMode: TextArea.Wrap
                }
            }
            
            Button {
                Layout.fillWidth: true
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                text: "🔄 Fetch from Suno"
                onClicked: {
                    console.log("Fetch lyrics from Suno")
                }
            }
            
            Button {
                Layout.fillWidth: true
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                text: "⏱️ Auto-sync Timestamps"
                onClicked: {
                    console.log("Auto-sync lyrics")
                }
            }
            
            Item { implicitHeight: 8 }
        }
    }
    
    Component {
        id: communitySectionComponent
        
        ColumnLayout {
            spacing: 12
            
            Label {
                Layout.leftMargin: 12
                text: "Community Presets"
                font.pixelSize: 12
                font.weight: Font.Medium
                color: themeManager.textColor
            }
            
            Button {
                Layout.fillWidth: true
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                text: "🔍 Browse Presets"
                onClicked: {
                    appController.gistUploader.fetchPublicPresets()
                }
            }
            
            Button {
                Layout.fillWidth: true
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                text: "📤 Share My Preset"
                onClicked: {
                    console.log("Share preset to Gist")
                }
            }
            
            Label {
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                text: "Share your creations with the community via GitHub Gists!"
                font.pixelSize: 11
                color: Qt.darker(themeManager.textColor, 1.4)
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            
            Item { implicitHeight: 8 }
        }
    }
}