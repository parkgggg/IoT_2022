// Create the charts when the web page loads
window.addEventListener('load', onload);

function onload(event){
  chartW = createsittingperhourChart();
  chartS = createshakingperhourChart();
}

// Create Weight Chart
function createsittingperhourChart() {
  var chart = new Highcharts.Chart({
    chart:{ 
      renderTo:'chart-sitting',
      type: 'column' 
    },
    series: [
      {
        name: '앉은 시간 (시간대별)'
      }
    ],
    title: { 
      text: undefined
    },
    plotOptions: {
      line: { 
        animation: false,
        dataLabels: { 
          enabled: true 
        }
      }
    },
    xAxis: {
      type: '시간대',
      dateTimeLabelFormats: { second: '%H:%M:%S' }
    },
    yAxis: {
      title: { 
        text: '앉아있었던 시간 (30초 당)' 
      }
    },
    credits: { 
      enabled: false 
    }
  });
  return chart;
}

// Create Shake Chart
function createshakingperhourChart(){
  var chart = new Highcharts.Chart({
    chart:{ 
      renderTo:'chart-shaking',
      type: 'column'  
    },
    series: [{
      name: '다리 떤 횟수 (시간대별)'
    }],
    title: { 
      text: undefined
    },    
    plotOptions: {
      line: { 
        animation: false,
        dataLabels: { 
          enabled: true 
        }
      },
      series: { 
        color: '#50b8b4' 
      }
    },
    xAxis: {
      type: '시간대',
      dateTimeLabelFormats: { second: '%H:%M:%S' }
    },
    yAxis: {
      title: { 
        text: '다리 떤 횟수 (30초 당)' 
      }
    },
    credits: { 
      enabled: false 
    }
  });
  return chart;
}
