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
   AddCommand(std::make_unique<CmdPlay>());
   AddCommand(std::make_unique<CmdPause>());
   AddCommand(std::make_unique<CmdStop>());

//    auto it = m_commands.begin();
//    for (; it != m_commands.end(); ++it)
//    {
//       Cmd* command = m_commands[i]->get();
//       int flags = command->Type();
//       wxItemKind kind =
//          flags & Cmd::CommandFlags::COMMAND_RADIO ? wxITEM_RADIO :
//          flags & Cmd::CommandFlags::COMMAND_TOGGLE ? wxITEM_CHECK : wxITEM_NORMAL;
// 
//       wxBitmap const& bitmap = command->Icon(16, GetLayoutDirection());
//       AddTool(500 + m_commands.size(), command->StrDisplay(m_context), bitmap, GetTooltip(command), kind);
// 
//       needs_onidle = needs_onidle || flags != Cmd::COMMAND_NORMAL;
//    }
   uint32_t i = 0;
   for (auto& cmd : m_commands) {
      Cmd* command = cmd.second.get();
      int flags = command->Type();
      wxItemKind kind =
         flags & Cmd::CommandFlags::COMMAND_RADIO ? wxITEM_RADIO :
         flags & Cmd::CommandFlags::COMMAND_TOGGLE ? wxITEM_CHECK : wxITEM_NORMAL;
      wxBitmap const& bitmap = command->Icon(16, GetLayoutDirection());
      command->SetToolID(500 + i);
      AddTool(500 + i, command->StrDisplay(m_context), bitmap, GetTooltip(command), kind);

      needs_onidle = needs_onidle || flags != Cmd::COMMAND_NORMAL;
      i++;
   }

   // Only bind the update function if there are actually any dynamic tools
   if (needs_onidle) {
      Bind(wxEVT_IDLE, &Toolbar::OnIdle, this);
   }

   Realize();
};

void Toolbar::AddCommand(std::unique_ptr<Cmd> ptr)
{
   m_commands[ptr->name()] = std::move(ptr);
}
