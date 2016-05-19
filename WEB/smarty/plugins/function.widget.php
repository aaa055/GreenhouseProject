<?php
/**
 * Smarty plugin
 * @package Smarty
 * @subpackage plugins
 */

/**
 * Smarty {widget} function plugin
 *
 * Type:     function<br>
 * Name:     widget<br>
 * Purpose:  echo widget data<br>
 * @author Porokhnya Dmitry <spywarrior@gmail.com>
 */
function smarty_function_widget($params, &$smarty)
{

	$arr_keys = array_keys($params);
	
	// регистрируем все переданные параметры в движке, чтобы шаблон имел возможность ими оперировать
	foreach($params as $key=>$val)
	{
    if($key != 'name' && $key !='number')
      $smarty->assign($key,$val);
	}

	if(!in_array('name',$arr_keys))
	{
		$smarty->trigger_error("widget: missing 'name' parameter");
		return "";
	} 

	if(!in_array('number',$arr_keys))
	{
		$smarty->trigger_error("widget: missing 'number' parameter");
		return "";
	} 
  
  return get_widget($params['name'],$params['number']);
}




/* vim: set expandtab: */

?>
