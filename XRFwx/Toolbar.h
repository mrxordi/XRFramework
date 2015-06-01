#pragma once
#include "wx/toolbar.h"
#include "Cmd.h"

struct Context;

class Toolbar : public wxToolBar
{
public:
   Toolbar(wxWindow *parent, std::string name, Context *c, std::string ht_context, bool vertical) : 
      wxToolBar(parent, -1, wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER | wxTB_FLAT | (vertical ? wxTB_VERTICAL : wxTB_HORIZONTAL)),
      m_context(c), m_name(std::move(name))
      {
      Populate();
      Bind(wxEVT_TOOL, &Toolbar::OnClick, this);
   };
   virtual ~Toolbar();


   /// Enable/disable the toolbar buttons
   void OnIdle(wxIdleEvent &) {
      for (size_t i = 0; i < m_commands.size(); ++i) {
         if (m_commands[i]->Type() & Cmd::CommandFlags::COMMAND_VALIDATE)
            EnableTool(500 + i, m_commands[i]->Validate(m_context));
         if (m_commands[i]->Type() & Cmd::CommandFlags::COMMAND_TOGGLE || m_commands[i]->Type() & Cmd::CommandFlags::COMMAND_RADIO)
            ToggleTool(500 + i, m_commands[i]->IsActive(m_context));
      }
   };

   /// Toolbar button click handler
   void OnClick(wxCommandEvent &evt) {
      (*m_commands[evt.GetId() - 500])(m_context);
   };

   /// Clear the toolbar and recreate it
   void RegenerateToolbar();

   wxString GetTooltip(Cmd *command) {
      wxString ret = command->StrHelp();
      return ret;
   };

   /// Populate the toolbar with buttons
   void Populate();

private:
   std::string m_name;
   std::vector<std::unique_ptr<Cmd> > m_commands;
   /// Project context
   Context *m_context;
};

