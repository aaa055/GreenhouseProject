<?php
// эта строчка обязательна к подключению всеми скриптами
require_once($_SERVER['DOCUMENT_ROOT'] . "/prerequisites.php");

if($authorized)
{
  $posted = intval(@$_GET['posted']);
  if($posted == 1)
  {
   $controller_id = intval(@$_GET['controller_id']);
   $sms_text = $dbengine->escapeString(mb_substr(strip_tags(@$_GET['sms_text']),0,50));
   $sms_answer = $dbengine->escapeString(mb_substr(strip_tags(@$_GET['sms_answer']),0,100));
   $sms_command = $dbengine->escapeString(mb_substr(strip_tags(@$_GET['sms_command']),0,255));
  
    if($controller_id > 0)
    {
      // обновляем базу
      $dbengine->beginTransaction();
      $dbengine->exec("DELETE FROM sms WHERE controller_id=$controller_id AND sms_text='$sms_text';");
      $dbengine->exec("INSERT INTO sms(controller_id,sms_text,sms_answer,sms_command) VALUES($controller_id,'$sms_text','$sms_answer','$sms_command');");
      $dbengine->commitTransaction();
    }   
      
  } // if
 
}


// создаём массив данных
$json_data = array('authorized' => $authorized);

// отсылаем его юзеру
echo json_encode($json_data);

?>