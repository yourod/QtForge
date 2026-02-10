import QtQml 2.15

QtObject {
    objectName: "rootObject"

    property QtObject bindingTarget: QtObject {
        objectName: "bindingTarget"
        property QtObject payload: null
        readonly property QtObject readOnlyPayload: null
    }

    property QtObject containerObject: QtObject {
        objectName: "containerObject"
        property QtObject payload: null
    }
}
