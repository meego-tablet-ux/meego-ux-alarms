import Qt 4.7
import MeeGo.Components 0.1

Window {
    fullScreen: true
    overlayItem: ModalDialog {
        showCancelButton: true
        showAcceptButton: true

        //: Title for the shutdown verification dialog that a user
        //: sees after doing a press-and-hold on the power button
        title: qsTr("Would you like to turn off now?")

        //: Cancel button to stop the shutdown from happening
        cancelButtonText: qsTr("Cancel")

        //: Ok button that will result in the device turning off
        acceptButtonText: qsTr("OK")
        
	onAccepted: {
            Qt.quit();
        }
        onRejected: {
            // TODO: Hook up to a qApp method for shutdown
            Qt.quit();
        }

        Component.onCompleted: show()
    }
}

