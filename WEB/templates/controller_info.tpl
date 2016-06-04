{* Smarty *}

<div  style='float:left;' id='controller{$controller.controller_id}'>
<fieldset class='controller_info_box ui-corner-all'>
<legend class='ui-widget-header ui-corner-all'><span id='controller_name{$controller.controller_id}'>{$controller.controller_name}</span></legend>

ID контроллера: <span id='controller_id{$controller.controller_id}'>{$controller.controller_id}</span><br/>
Адрес контроллера: <span id='controller_address{$controller.controller_id}'>{$controller.controller_address}</span><br/>
Онлайн: <b><span id='controller_status{$controller.controller_id}'>{if $controller.is_online==1}<span class='auto_mode'>да</span>{else}<span class='manual_mode'>нет</span>{/if}</span></b><br/>

<div style='margin-top:8px;'>

<table width='100%' border='0' cellspacing='0' cellpadding='2'>
<tr>
<td width='50%'>
    <a href="javascript:editController({$controller.controller_id});" id='controller_edit_link{$controller.controller_id}'>Редактировать</a>
</td>
<td width='50%'>

    <a href="controller.php?id={$controller.controller_id}" id='controller_view_link{$controller.controller_id}'>Показания</a>
</td>

</tr>
<tr>
<td width='50%'>
    <a href="controller_settings.php?id={$controller.controller_id}" id='controller_settings_link{$controller.controller_id}'>Настройки</a>
</td>
<td width='50%'>
    <a href="controller_charts.php?id={$controller.controller_id}" id='controller_charts_link{$controller.controller_id}'>Графики</a>
</td>
</tr>
</table>
</div>

</fieldset>
</div>

<script type='text/javascript'>
$(document).ready(function(){ldelim}

  $( "#controller_settings_link{$controller.controller_id}" ).button({ldelim}
      icons: {ldelim}
        primary: "ui-icon-gear"
      {rdelim}
    {rdelim}).css("width","100%");
    
  $( "#controller_edit_link{$controller.controller_id}" ).button({ldelim}
      icons: {ldelim}
        primary: "ui-icon-pencil"
      {rdelim}
    {rdelim}).css("width","100%");
    
  $( "#controller_view_link{$controller.controller_id}" ).button({ldelim}
      icons: {ldelim}
        primary: "ui-icon-info"
      {rdelim}
    {rdelim}).css("width","100%");  
    
  $( "#controller_charts_link{$controller.controller_id}" ).button({ldelim}
      icons: {ldelim}
        primary: "ui-icon-image"
      {rdelim}
    {rdelim}).css("width","100%");         
        

{rdelim});  
</script>
