import Qt 4.7
import MeeGo.Components 0.1

Window {
    fullScreen: true
    overlayItem: ModalDialog {
        showCancelButton: true
        showAcceptButton: currentRequest.snooze

        //: Title for the alarm clock alarm dialog box
        title: qsTr("Alarm")

        //: Button text to make the alarm stop without snoozing
        cancelButtonText: qsTr("OK")

        //: Button text to snooze the alarm
        acceptButtonText: qsTr("Snooze")

        content: Text {
            anchors.centerIn: parent
            width: parent.width - 20
            wrapMode: Text.Wrap
            text: currentRequest.body
        }
        onAccepted: {
            // In order to snooze we just close the dialog and alow the libealarm mechanism
            // trigger the alarm dialog again in the future
            Qt.quit();
        }
        onRejected: {
            qApp.cancelAlarm();
            Qt.quit();
        }

        Component.onCompleted: show()
    }
}

