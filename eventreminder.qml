import Qt 4.7
import MeeGo.Components 0.1

Window {
    fullScreen: true
    overlayItem: ModalDialog {
        showCancelButton: true
        showAcceptButton: true

        //: Title for the calendar event reminder dialog box
        title: qsTr("Event")

        //: Button text to make the event reminder dialog go away
        //: without opening the calendar application
        cancelButtonText: qsTr("OK")

        //: Button text to make the event reminder dialog open the
        //: calendar application viewing the event in question
        acceptButtonText: qsTr("View")

        content: Text {
            anchors.centerIn: parent
            width: parent.width - 20
            wrapMode: Text.Wrap
            text: currentRequest.body
        }
        onAccepted: {
            qApp.triggerAction(currentRequest.acceptAction);
            Qt.quit();
        }
        onRejected: {
            qApp.triggerAction(currentRequest.rejectAction);
            Qt.quit();
        }

        Component.onCompleted: show()
    }
}

