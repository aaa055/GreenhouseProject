{* Smarty *}
{include file='controller_head.tpl' additional_text=', графики'}

<!--[if lte IE 8]><script language="javascript" type="text/javascript" src="js/excanvas.min.js"></script><![endif]-->
<script language="javascript" type="text/javascript" src="js/jquery.flot.js"></script>
<script language="javascript" type="text/javascript" src="js/jquery.flot.time.js"></script>


<div id="data_requested_dialog" title="Обработка данных..." class='hdn'>
  <p>Пожалуйста, подождите, пока данные обрабатываются...</p>
</div>


<div class='ui-widget ui-widget-content ui-corner-all padding_around8px' style='margin-bottom:8px;text-align:right;'>
 Показать графики за:
  <select id='interval_select' onchange="doRequest(this);">
    <option value="0">Последний час</option>
    <option value="1" selected>Текущий день</option>
    <option value="2">Последние сутки</option>
    <option value="3">Неделю</option>
    <option value="4">Месяц</option>
  </select>
  
  <button id='refresh_button'>Обновить</button>

</div>


<div id='series_buttons'>

   <div style='text-align:right;padding:4px;'>
    Дискретность: 
      <select id='ticks_select' onchange="changeTicks(this);">
        <option value='15,minute'>15 минут</option>
        <option value='30,minute'>30 минут</option>
        <option value='1,hour' selected>1 час</option>
        <option value='2,hour'>2 часа</option>
        <option value='1,day'>день</option>
        <option value='1,week'>неделя</option>
        <option value='1,month'>месяц</option>
      </select>
   </div>

  <ul id='series_buttons_ul'>
  </ul>
</div>

<div id='no_data' class='hdn'>
  <div class='ui-state-error ui-corner-all error'><img src='images/logo48.png' class='logo'>
    За указанный период показания с датчиков контроллера отсутствуют.
  </div>
</div>



<script type='text/javascript'>
var controller = new Controller({$selected_controller.controller_id},'{$selected_controller.controller_name}','{$selected_controller.controller_address}',true);

{literal}

function doRequest(obj)
{
  var val = parseInt($(obj).val());
  
  var to = new Date();
  var from = new Date();
  
    
  switch(val)
  {
    case 0: // последний час
      from.setHours(to.getHours()-1);
    break;
    
    case 1: // текущий день
      from.setHours(0);
      from.setMinutes(0);
    break;
    
    case 2: // Последние сутки
      from.setHours(to.getHours()-24);
    break;
    
    case 3: // Неделю
      from.setDate(to.getDate()-7);
    break;
    
    case 4: // Месяц
      from.setDate(to.getDate()-30);
    break;
  }
  
  requestStatsData(from,to);
}

function changeTicks(obj)
{
  var val = $(obj).val().toString().split(",");
  var duration = val[0];
  var period = val[1]; 
  
  for(var i=0;i<plots.length;i++)
  {
    var plot = plots[i];
    plot.getOptions().xaxes[0].tickSize[0] = duration;
    plot.getOptions().xaxes[0].tickSize[1] = period;
    plot.setupGrid();
    plot.draw();
  }
}


var chartSeries = new Object();
var plots = new Array();

function showChartForSerie(serieType,serie)
{
  var chartBox = $('#' + serieType + '_chart');
  
  var data = new Array();
  
        for(var moduleName in serie.modules)
        {
          
            var module = serie.modules[moduleName];
            
            var moduleData = module.data;
            
            
            for(var sensorIndex in moduleData)
            {
              var dataSerie = new Object();
              dataSerie.label = module.moduleDescription + ', ' + sensorIndex;
              dataSerie.data = moduleData[sensorIndex];
              dataSerie.serieType = serieType;
              dataSerie.clickable = true;
              dataSerie.hoverable = true;
              dataSerie.myLabel = dataSerie.label; // плагин threshold неверно работает с label, поэтому копируем это свойство в наше
              
              var unit = '';
              
              switch(serieType)
              {
                case 'TEMP':
                  unit = ' &#0176;C';
                break;
                
                case 'HUMIDITY':
                case 'SOIL':
                  unit = '%';
                break;
                
                case 'LIGHT':
                  unit = ' люкс';
                break;
                
                case 'PH':
                  unit = ' pH';
                break;
                
                case 'FLOW_INSTANT':
                case 'FLOW_INCREMENTAL':
                  unit = ' л.';
                break;
              }
              
              dataSerie.unit = unit;
             

              
              data.push(dataSerie);
            

            } // for
            
        } // for  

  var options = {
    series: {
        lines: { show: true},
        points: { show: false}
        
    }
    
    , legend: { noColumns : 4, sorted: true, margin : 4, backgroundOpacity: 0, container : "#" + serieType + '_legend' }
    , xaxis: {mode : 'time', timezone: 'browser', timeformat: "%d.%m.%Y %H:%M", tickSize: [1, "hour"]}
    , grid: { hoverable: true, clickable: false }
};
  
  var pl = $.plot(chartBox, data, options);
  
  chartBox.bind("plothover", function (event, pos, item) 
          {

            if (item) 
            {
              var y = item.datapoint[1].toFixed(2);
              
              $("#tooltip").html(item.series.myLabel + "<br/><div style='margin-top:8px;font-weight:bold;'>" + y + item.series.unit + '</div>')
                .css({top: item.pageY+5, left: item.pageX+5})
                .fadeIn(200);
            } 
            else 
            {
              $("#tooltip").hide();
            }
			
		});  
  
  plots.push(pl);
}
var __tabsInited = false;

function requestStatsData(fromDate,toDate)
{
  var f = parseInt(fromDate.getTime()/1000);
  var t = parseInt(toDate.getTime()/1000);

  $("#data_requested_dialog" ).dialog({
                dialogClass: "no-close",
                modal: true,
                closeOnEscape: false,
                draggable: false,
                resizable: false,
                buttons: []
              });

  controller.queryServerScript('/x_query_stats.php',{from: f, to: t},function(c,queryResult) {
      
    $("#data_requested_dialog" ).dialog('close');  
      
    $('#series_buttons').show();
    $('#no_data').toggle(!queryResult.data.length);
    
    if(!queryResult.data.length)
    {
      $('#series_buttons').hide();
      return;
    }
    
    plots = new Array();

    // теперь назначаем имена датчикам, если они есть в системе
    for(var i=0;i<queryResult.data.length;i++)
    {
      queryResult.data[i].i = controller.SensorsNames.getMnemonicName(new Sensor(queryResult.data[i].i,queryResult.data[i].m));
    } // for
    
    // на данный момент имеем массив данных всех датчиков, упорядоченный по типам показаний.
    // нам надо сделать серии данных для каждого из типов показаний
    chartSeries = new Object();
    for(var i=0;i<queryResult.data.length;i++)
    {
      var row = queryResult.data[i];
      if(!(chartSeries[row.t] != undefined))
      {
        chartSeries[row.t] = new Object(); // создаём новую серию для типа показаний, например, "Температура"
        var serie = chartSeries[row.t];
        serie.serieName = row.d; // легенда серии
        serie.modules = new Object(); // данные с модулей в серии
      }
       
       var serie = chartSeries[row.t];
       if(!(serie.modules[row.m] != undefined))
       {
          serie.modules[row.m] = new Object(); // если не существует набор данных для модуля, то мы его создаём
          serie.modules[row.m].moduleDescription = row.md;
          serie.modules[row.m].data = new Object(); // массив показаний с датчиков определённого модуля
       }
       
       var dataHolder = serie.modules[row.m].data;
       
       if(!(dataHolder[row.i] != undefined))
          dataHolder[row.i] = new Array();
       
       // пишем данные в серию для датчика
       dataHolder[row.i].push(new Array(row.rd,row.sd));
        
    } // for
    
    // создаём нужные кнопки
    var tabsList = $('#series_buttons_ul');
    
    for(var serieType in chartSeries)
    {
      var serie = chartSeries[serieType];
      if(!__tabsInited)
      {
        var li = $('<li/>', {id: 'tab' + serieType}).appendTo(tabsList);
        var link = $('<a/>',{href: '#' + serieType }).appendTo(li).text(serie.serieName);
        var chartBox = $('<div/>', {id : serieType }).appendTo('#series_buttons');
        $('<div/>', {id : serieType + '_legend'}).appendTo(chartBox).css('margin-bottom','10px');
        $('<div/>', {id : serieType + '_chart'}).appendTo(chartBox).css('width',$('#series_buttons').width() - 50).css('height','450px');
      }
              
      showChartForSerie(serieType,serie);
      
      
    } // for
    
    if(!__tabsInited) 
    {
      __tabsInited = true;    
      var sortedTabs = $('#series_buttons').find("li").sort(function (a, b) {
        
        var atxt = $(a).find('a').text();
        var btxt = $(b).find('a').text();
        
        if (atxt < btxt) 
            return -1;
        else
            return 1;
    });

      $('#series_buttons').find('ul').html(sortedTabs);    
      $('#series_buttons').tabs({active : 0});
      
    } // if
    
    $('#ticks_select').trigger('change');
    
  
  });
}

controller.OnGetSensorNames = function(c)
{
  $('#interval_select').trigger('change');

};

$(document).ready(function(){


$("<div id='tooltip'></div>").css({
			position: "absolute",
			display: "none",
			border: "1px solid #fdd",
			padding: "2px",
			"background-color": "#fee",
			opacity: 0.80
		}).appendTo("body");


  $('#refresh_button').button({
      icons: {
        primary: "ui-icon-refresh"
      }
      
    }).click(function(ev) {
        $('#interval_select').trigger('change');
      });

  controller.querySensorNames(); // запрашиваем список имён датчиков из БД
  


});
{/literal}

</script>
