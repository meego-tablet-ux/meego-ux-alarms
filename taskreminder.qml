import Qt 4.7
import MeeGo.Components 0.1

Window {
    fullScreen: true
    overlayItem: ModalDialog {
        showCancelButton: true
        showAcceptButton: true

        //: Title for the task reminder dialog box
        title: qsTr("Task")

        //: Button text to make the task reminder dialog go away
        //: without opening the task application
        cancelButtonText: qsTr("OK")

        //: Button text to make the task reminder dialog open the
        //: task application viewing the task in question
        acceptButtonText: qsTr("View")

        content: Text {
            anchors.centerIn: parent
            width: parent.width - 20
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            text: currentRequest.body
        }
        onAccepted: {
            qApp.launchDesktopByName("/usr/share/applications/meego-app-tasks.desktop", "openTasks", currentRequest.uid);
            Qt.quit();
        }
        onRejected: {            
            qApp.cancelAlarm();
            Qt.quit();
        }

        Component.onCompleted: show()
    }
}

