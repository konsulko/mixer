/*
 * Copyright 2016 Konsulko Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import QtQuick 2.6
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0
import AGL.Demo.Controls 1.0
import PaControlModel 1.0

ApplicationWindow {
	id: root

	Label { 
		id: title
		font.pixelSize: 48
		text: "Mixer"
		anchors.horizontalCenter: parent.horizontalCenter
	}

	Component {
		id: ctldesc
		Label {
			font.pixelSize: 32
			width: listView.width
			wrapMode: Text.WordWrap
			property var typeString: {modelType ? "Output" : "Input"}
			text: "[" + typeString + " #" + modelCIndex + "]: " + modelDesc
		}
	}

	Component {
		id: empty
		Item {
		}
	}

	ListView {
		id: listView
		anchors.left: parent.left
		anchors.top: title.bottom
		anchors.margins: 80
		anchors.fill: parent
		model: PaControlModel {}
		delegate: ColumnLayout {
			width: parent.width
			spacing: 40
			Loader {
				property int modelType: type
				property int modelCIndex: cindex
				property string modelDesc: desc
				sourceComponent: (channel == 0) ? ctldesc : empty
			}
			RowLayout {
				Layout.minimumHeight: 75
				Label {
					font.pixelSize: 24
					text: cdesc
					Layout.minimumWidth: 150
				}
				Label {
					font.pixelSize: 24
					text: "0 %"
				}
				Slider {
					Layout.fillWidth: true
					from: 0
					to: 65536
					stepSize: 256
					snapMode: Slider.SnapOnRelease
					onValueChanged: volume = value
					Component.onCompleted: value = volume
				}
				Label {
					font.pixelSize: 24
					text: "100 %"
				}
			}
		}
	}
}
