
// Create Sitting Gauge
function createSittingGauge() {
    var gauge = new LinearGauge({
        renderTo: 'gauge-sitting',
        width: 120,
        height: 400,
        units: "앉아있던 시간 (지난 30초)",
        minValue: 0,
        startAngle: 90,
        ticksAngle: 180,
        maxValue: 60,
        colorValueBoxRect: "#049faa",
        colorValueBoxRectEnd: "#049faa",
        colorValueBoxBackground: "#f1fbfc",
        valueDec: 2,
        valueInt: 2,
        majorTicks: [
            "0",
            "10",
            "20",
            "30",
            "40",
            "50",
            "60"
        ],
        minorTicks: 4,
        strokeTicks: true,
        highlights: [
            {
                "from": 50,
                "to": 60,
                "color": "rgba(200, 50, 50, .75)"
            }
        ],
        colorPlate: "#fff",
        colorBarProgress: "#CC2936",
        colorBarProgressEnd: "#049faa",
        borderShadowWidth: 0,
        borders: false,
        needleType: "arrow",
        needleWidth: 2,
        needleCircleSize: 7,
        needleCircleOuter: true,
        needleCircleInner: false,
        animationDuration: 1500,
        animationRule: "linear",
        barWidth: 10,
    });
    return gauge;
}


// Create Shaking Gauge
function createShakingGauge(){
    var gauge = new RadialGauge({
        renderTo: 'gauge-shaking',
        width: 300,
        height: 300,
        units: "다리 떤 횟수 (지난 30초)",
        minValue: 0,
        maxValue: 100,
        colorValueBoxRect: "#049faa",
        colorValueBoxRectEnd: "#049faa",
        colorValueBoxBackground: "#f1fbfc",
        valueInt: 2,
        majorTicks: [
            "0",
            "20",
            "40",
            "60",
            "80",
            "100"
    
        ],
        minorTicks: 4,
        strokeTicks: true,
        highlights: [
            {
                "from": 80,
                "to": 100,
                "color": "#03C0C1"
            }
        ],
        colorPlate: "#fff",
        borderShadowWidth: 0,
        borders: false,
        needleType: "line",
        colorNeedle: "#007F80",
        colorNeedleEnd: "#007F80",
        needleWidth: 2,
        needleCircleSize: 3,
        colorNeedleCircleOuter: "#007F80",
        needleCircleOuter: true,
        needleCircleInner: false,
        animationDuration: 1500,
        animationRule: "linear"
    });
    return gauge;
}
