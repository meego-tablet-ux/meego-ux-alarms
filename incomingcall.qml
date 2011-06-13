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

        content: Row{
            width: parent.width - 20
            height: childrenRect.height
            spacing: 20
            anchors.centerIn: parent

            Image {
                source: currentRequest.imageURI
                height: 70
                width: 70
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                wrapMode: Text.Wrap
                text: currentRequest.body
            }
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

