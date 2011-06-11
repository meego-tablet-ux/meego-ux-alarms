import Qt 4.7
import MeeGo.Components 0.1

Window {
    fullScreen: true
    overlayItem: ModalDialog {
        showCancelButton: true
        showAcceptButton: true

        //: Title for the incoming call dialog box
        title: qsTr("Incoming call")

        //: Decline answering the incoming VOIP call
        cancelButtonText: qsTr("Decline")

        //: Answer the incoming VOIP call
        acceptButtonText: qsTr("Accept")

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

