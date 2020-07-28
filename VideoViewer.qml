import QtQuick 2.9
import QtQuick.Controls 2.2
import QtMultimedia 5.9
import QtQuick.Layouts 1.3

Page {
    id: root
    property alias source: video.source
    property alias nameText: nameLabel.text

    padding: 0
    background: Rectangle {
        //anchors.fill: parent
        color: "black"
        //visible: !root.source.connected
    }

    VideoOutput {
        id: video
        anchors.fill: parent
        //source: cameraViewer
        visible: source.connected
    }
    ColumnLayout {

        anchors.centerIn: parent
        visible: !root.source.connected

        Label {
            text: "\uEB6C" //icon-eye-off-1
            font.family: "fontello"
            font.pointSize: 30
            horizontalAlignment: Label.AlignHCenter
            verticalAlignment: Label.AlignVCenter
            //color: "white"
            Layout.alignment: Qt.AlignCenter
        }

        Label {
            text: "Камера недоступна"
            horizontalAlignment: Label.AlignHCenter
            verticalAlignment: Label.AlignVCenter
            //color: "white"
            Layout.alignment: Qt.AlignCenter
        }
    }

    Label {
        id: nameLabel
        anchors.left: parent.left
        //anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 4

        padding: 5

        //text: "Имя не указано"
        //color: "white"

        background: Rectangle {
            color: "#88CCCCCC"
            radius: 1
        }

        visible: !!nameLabel.text
    }
}
