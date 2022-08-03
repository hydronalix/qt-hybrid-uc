import QtQuick 2.2

Item {
    id: valueSource
    property real kph: 0
    property real rpm: 1
    property bool start: true

    property int test: 50
    property int prevTest: 30

    SequentialAnimation {
        running: true
        loops: 1

        PropertyAction {
            target: valueSource
            property: "start"
            value: false
        }

        SequentialAnimation {
            loops: Animation.Infinite

            ParallelAnimation {
                NumberAnimation {
                    target: valueSource
                    property: "kph"
                    easing.type: Easing.InOutSine
                    from: prevTest
                    to: test
                    duration: 3000
                }
            }
        }
    }
}
