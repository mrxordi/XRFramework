#include "stdafxf.h"
#include "Toolbar.h"


Toolbar::~Toolbar()
{
}

void Toolbar::RegenerateToolbar() {
   Unbind(wxEVT_IDLE, &Toolbar::OnIdle, this);
   ClearTools();
   m_commands.clear();
   Populate();
};

void Toolbar::Populate() {
   bool needs_onidle = false;
   Cmd* arr_command[2];
   arr_command[0] = static_cast<Cmd*>(new CmdPlay());
   arr_command[1] = static_cast<Cmd*>(new CmdPause());
   arr_command[2] = static_cast<Cmd*>(new CmdStop());

   for (size_t i = 0; i < 3; i++)
   {
      Cmd* command = arr_command[i];
      int flags = command->Type();
      wxItemKind kind =
         flags & Cmd::CommandFlags::COMMAND_RADIO ? wxITEM_RADIO :
         flags & Cmd::CommandFlags::COMMAND_TOGGLE ? wxITEM_CHECK : wxITEM_NORMAL;

      wxBitmap const& bitmap = command->Icon(16, GetLayoutDirection());
      AddTool(500 + m_commands.size(), command->StrDisplay(m_context), bitmap, GetTooltip(command), kind);

      m_commands.push_back(std::unique_ptr<Cmd>(command));

      needs_onidle = needs_onidle || flags != Cmd::COMMAND_NORMAL;
   }

   // Only bind the update function if there are actually any dynamic tools
   if (needs_onidle) {
      Bind(wxEVT_IDLE, &Toolbar::OnIdle, this);
   }

   Realize();
};