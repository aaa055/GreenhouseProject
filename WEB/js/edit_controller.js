//-----------------------------------------------------------------------------------------------------
// редактируем данные контроллера
function editController(controller_id)
{
  var controller = null;
  for(var i=0;i<controllers.length;i++)
  {
    if(controllers[i].getId() == controller_id)
    {
      controller = controllers[i];
      break;
    }
  }
  
  if(!controller)
    return;
  
  $("#controller_name").val(controller.getName()); 
  $("#controller_address").val(controller.getAddress()); 
  $("#edit_controller_id").val(controller.getId()); 
  
  $("#controller_dialog").dialog({modal:true, buttons: [{text: "Изменить", click: function(){
  
    var old_id = controller.getId();
    var cat_name = $("#controller_name").val();
    var c_addr =  $("#controller_address").val();
    var new_id =  $("#edit_controller_id").val();
    
    if(cat_name != '' && c_addr != '')
    {
      $(this).dialog("close");
      
      controller.setName(cat_name);
      controller.setAddress(c_addr);
      
      $( "#controller_in_process_dialog" ).dialog({
                dialogClass: "no-close",
                modal: true,
                closeOnEscape: false,
                draggable: false,
                resizable: false,
                buttons: []
              });
              
       controller.edit(new_id,function(obj,answer){
       
          $( "#controller_in_process_dialog" ).dialog('close');

          $("#controller_name" + old_id).html(cat_name);
          $("#controller_address" + old_id).html(c_addr);
          $("#controller_id" + old_id).html(new_id);

              if(new_id != old_id && new_id != '')
              {
                $("#controller" + old_id).attr("id","controller" + new_id);

                $("#controller_name" + old_id).attr("id","controller_name" + new_id);
                $("#controller_address" + old_id).attr("id","controller_address" + new_id);
                $("#controller_id" + old_id).attr("id","controller_id" + new_id);
                $("#controller_status" + old_id).attr("id","controller_status" + new_id);

                $("#controller_view_link" + old_id).attr("href","controller.php?id=" + new_id);
                $("#controller_settings_link" + old_id).attr("href","controller_settings.php?id=" + new_id);

                $("#controller_edit_link" + old_id).attr("href","javascript:editController(" + new_id + ");");
                $("#controller_edit_link" + old_id).attr("id","controller_edit_link" + new_id);
              }
   
   
       
       });           
         
    } // cat_name != ''
  
  } }
  
  , {text: "Отмена", click: function(){$(this).dialog("close");} }
  ] });   

  
}
//-----------------------------------------------------------------------------------------------------
