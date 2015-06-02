#pragma once
#include "XRCommon/log/Log.h"
#include "wx/bitmap.h"
#include "wx/string.h"
#include "wx/intl.h"
#include "wx/buffer.h"
#include "wx/mstream.h"

const unsigned char button_play_16[] = { 137, 80, 78, 71, 13, 10, 26, 10, 0, 0, 0, 13, 73, 72, 68, 82, 0, 0, 0, 16, 0, 0, 0, 16, 8, 4, 0, 0, 0, 181, 250, 55, 234, 0, 0, 0, 57, 73, 68, 65, 84, 40, 145, 99, 96, 24, 108, 96, 1, 131, 48, 126, 5, 199, 24, 46, 48, 72, 227, 83, 176, 159, 225, 63, 195, 53, 6, 53, 252, 10, 254, 51, 220, 102, 112, 38, 164, 192, 130, 108, 43, 142, 49, 92, 196, 239, 200, 213, 132, 188, 57, 48, 0, 0, 131, 24, 17, 244, 230, 159, 99, 140, 0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130 };
const unsigned char button_play_24[] = { 137, 80, 78, 71, 13, 10, 26, 10, 0, 0, 0, 13, 73, 72, 68, 82, 0, 0, 0, 24, 0, 0, 0, 24, 8, 4, 0, 0, 0, 74, 126, 245, 115, 0, 0, 0, 78, 73, 68, 65, 84, 56, 141, 99, 96, 24, 5, 228, 129, 2, 6, 125, 210, 52, 92, 102, 248, 204, 96, 68, 138, 134, 93, 12, 255, 25, 94, 48, 216, 145, 166, 225, 63, 195, 43, 6, 95, 210, 52, 252, 103, 120, 205, 16, 71, 154, 134, 255, 12, 111, 25, 26, 72, 213, 80, 77, 67, 39, 145, 232, 105, 146, 130, 149, 228, 136, 43, 37, 53, 105, 140, 2, 116, 0, 0, 106, 209, 41, 17, 48, 14, 44, 234, 0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130 };
const unsigned char button_play_32[] = { 137, 80, 78, 71, 13, 10, 26, 10, 0, 0, 0, 13, 73, 72, 68, 82, 0, 0, 0, 32, 0, 0, 0, 32, 8, 4, 0, 0, 0, 217, 115, 178, 127, 0, 0, 0, 89, 73, 68, 65, 84, 72, 199, 99, 96, 24, 5, 163, 0, 31, 216, 192, 32, 70, 153, 1, 255, 129, 80, 159, 82, 3, 254, 51, 184, 81, 106, 192, 127, 134, 100, 74, 13, 248, 207, 208, 65, 169, 1, 255, 24, 150, 80, 102, 0, 200, 136, 3, 148, 25, 0, 130, 123, 41, 53, 128, 155, 50, 3, 152, 41, 243, 2, 69, 177, 240, 155, 178, 116, 240, 153, 178, 148, 248, 146, 178, 188, 112, 115, 180, 72, 26, 5, 116, 3, 0, 104, 194, 73, 194, 197, 216, 194, 177, 0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130 };
const unsigned char button_play_48[] = { 137, 80, 78, 71, 13, 10, 26, 10, 0, 0, 0, 13, 73, 72, 68, 82, 0, 0, 0, 48, 0, 0, 0, 48, 8, 4, 0, 0, 0, 253, 11, 49, 12, 0, 0, 0, 197, 73, 68, 65, 84, 88, 195, 237, 212, 177, 10, 65, 97, 24, 135, 241, 135, 12, 58, 6, 25, 68, 54, 27, 199, 130, 137, 217, 102, 83, 174, 194, 69, 184, 2, 139, 157, 114, 3, 22, 155, 65, 41, 177, 89, 212, 41, 22, 6, 110, 226, 164, 206, 107, 151, 241, 252, 7, 245, 62, 23, 240, 253, 250, 250, 190, 247, 5, 207, 243, 188, 255, 41, 79, 69, 123, 252, 145, 41, 69, 29, 16, 98, 196, 236, 117, 183, 8, 49, 140, 55, 115, 154, 74, 192, 136, 89, 17, 42, 1, 35, 225, 66, 71, 9, 24, 198, 150, 150, 22, 48, 14, 12, 201, 40, 129, 132, 136, 113, 154, 196, 55, 96, 36, 220, 232, 146, 213, 1, 134, 241, 96, 66, 160, 4, 140, 43, 35, 45, 240, 162, 167, 4, 158, 12, 148, 143, 124, 167, 175, 252, 69, 17, 109, 229, 28, 156, 105, 40, 39, 249, 68, 93, 185, 42, 118, 212, 148, 203, 110, 67, 25, 116, 192, 154, 18, 232, 128, 5, 5, 80, 1, 75, 102, 105, 237, 157, 95, 5, 84, 201, 225, 121, 158, 247, 183, 125, 0, 13, 57, 154, 239, 117, 191, 28, 200, 0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130 };
const unsigned char button_play_64[] = { 137, 80, 78, 71, 13, 10, 26, 10, 0, 0, 0, 13, 73, 72, 68, 82, 0, 0, 0, 64, 0, 0, 0, 64, 8, 4, 0, 0, 0, 0, 96, 185, 85, 0, 0, 0, 212, 73, 68, 65, 84, 104, 222, 237, 215, 177, 10, 65, 81, 28, 199, 241, 47, 162, 164, 76, 50, 202, 27, 144, 108, 22, 147, 205, 102, 51, 26, 141, 6, 175, 96, 32, 147, 209, 34, 163, 205, 100, 179, 123, 2, 121, 0, 41, 27, 3, 225, 254, 221, 219, 125, 5, 245, 75, 247, 255, 61, 221, 253, 211, 169, 115, 207, 255, 128, 231, 121, 158, 231, 121, 222, 31, 86, 103, 74, 89, 9, 176, 112, 29, 169, 104, 1, 198, 153, 154, 22, 96, 92, 104, 104, 1, 198, 157, 142, 22, 96, 60, 233, 105, 1, 198, 155, 129, 22, 96, 4, 140, 180, 128, 104, 77, 180, 128, 32, 252, 22, 164, 149, 59, 16, 17, 214, 100, 117, 128, 152, 176, 37, 175, 3, 196, 132, 61, 69, 29, 32, 94, 7, 74, 90, 128, 177, 74, 56, 96, 71, 65, 9, 216, 144, 83, 30, 195, 37, 25, 229, 49, 156, 145, 82, 254, 136, 198, 202, 187, 224, 195, 80, 121, 25, 189, 232, 43, 175, 227, 7, 93, 229, 64, 114, 163, 173, 28, 201, 174, 52, 19, 60, 150, 159, 168, 38, 246, 105, 214, 98, 174, 125, 156, 122, 158, 231, 121, 158, 231, 253, 180, 47, 147, 68, 22, 52, 51, 97, 10, 180, 0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130 };
const unsigned char button_pause_16[] = { 137, 80, 78, 71, 13, 10, 26, 10, 0, 0, 0, 13, 73, 72, 68, 82, 0, 0, 0, 16, 0, 0, 0, 16, 8, 4, 0, 0, 0, 181, 250, 55, 234, 0, 0, 0, 54, 73, 68, 65, 84, 40, 207, 99, 96, 24, 76, 96, 30, 195, 54, 134, 163, 12, 106, 64, 120, 20, 200, 90, 130, 169, 96, 63, 195, 127, 32, 180, 1, 66, 16, 125, 124, 136, 42, 88, 202, 112, 128, 225, 28, 216, 155, 231, 128, 172, 53, 131, 41, 6, 0, 86, 105, 45, 26, 29, 200, 24, 179, 0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130 };
const unsigned char button_pause_24[] = { 137, 80, 78, 71, 13, 10, 26, 10, 0, 0, 0, 13, 73, 72, 68, 82, 0, 0, 0, 24, 0, 0, 0, 24, 8, 4, 0, 0, 0, 74, 126, 245, 115, 0, 0, 0, 68, 73, 68, 65, 84, 56, 79, 99, 96, 24, 5, 164, 130, 66, 134, 93, 12, 91, 128, 112, 57, 152, 183, 8, 204, 222, 5, 20, 197, 9, 14, 48, 252, 7, 195, 215, 96, 222, 11, 40, 239, 0, 110, 13, 187, 160, 74, 30, 131, 121, 247, 161, 188, 93, 163, 26, 232, 172, 129, 228, 136, 35, 57, 105, 140, 2, 236, 0, 0, 248, 92, 104, 249, 234, 152, 211, 110, 0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130 };
const unsigned char button_pause_32[] = { 137, 80, 78, 71, 13, 10, 26, 10, 0, 0, 0, 13, 73, 72, 68, 82, 0, 0, 0, 32, 0, 0, 0, 32, 8, 4, 0, 0, 0, 217, 115, 178, 127, 0, 0, 0, 42, 73, 68, 65, 84, 72, 199, 99, 96, 24, 5, 163, 0, 27, 216, 192, 240, 31, 14, 113, 137, 224, 5, 255, 49, 148, 255, 31, 53, 96, 212, 128, 81, 3, 6, 196, 128, 81, 48, 10, 200, 0, 0, 3, 90, 178, 175, 90, 223, 0, 204, 0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130 };
const unsigned char button_pause_48[] = { 137, 80, 78, 71, 13, 10, 26, 10, 0, 0, 0, 13, 73, 72, 68, 82, 0, 0, 0, 48, 0, 0, 0, 48, 8, 4, 0, 0, 0, 253, 11, 49, 12, 0, 0, 0, 102, 73, 68, 65, 84, 88, 133, 237, 141, 177, 13, 128, 48, 12, 4, 61, 68, 164, 236, 224, 44, 75, 203, 4, 84, 76, 64, 201, 10, 48, 15, 116, 86, 176, 5, 34, 72, 32, 138, 187, 47, 79, 250, 19, 1, 0, 248, 55, 73, 244, 176, 84, 57, 189, 112, 183, 239, 55, 183, 193, 92, 231, 204, 36, 165, 61, 160, 33, 176, 154, 91, 130, 35, 64, 128, 0, 1, 2, 4, 8, 188, 23, 152, 221, 73, 111, 110, 12, 129, 220, 30, 200, 82, 220, 206, 221, 131, 123, 0, 128, 47, 217, 1, 165, 190, 142, 129, 236, 133, 204, 227, 0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130 };
const unsigned char button_pause_64[] = { 137, 80, 78, 71, 13, 10, 26, 10, 0, 0, 0, 13, 73, 72, 68, 82, 0, 0, 0, 64, 0, 0, 0, 64, 8, 4, 0, 0, 0, 0, 96, 185, 85, 0, 0, 0, 69, 73, 68, 65, 84, 104, 222, 237, 206, 177, 13, 0, 32, 8, 69, 65, 182, 118, 5, 182, 198, 5, 176, 192, 74, 147, 59, 218, 31, 242, 34, 0, 0, 62, 176, 162, 154, 91, 227, 205, 181, 58, 220, 116, 35, 64, 128, 0, 1, 2, 4, 8, 16, 32, 64, 128, 0, 1, 2, 254, 13, 200, 246, 117, 142, 55, 0, 0, 111, 218, 211, 17, 201, 83, 185, 60, 86, 213, 0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130 };
const unsigned char button_stop_16[] = { 137, 80, 78, 71, 13, 10, 26, 10, 0, 0, 0, 13, 73, 72, 68, 82, 0, 0, 0, 16, 0, 0, 0, 16, 8, 4, 0, 0, 0, 181, 250, 55, 234, 0, 0, 0, 38, 73, 68, 65, 84, 40, 83, 99, 96, 24, 76, 96, 30, 195, 54, 36, 184, 4, 211, 105, 251, 25, 254, 35, 193, 227, 67, 84, 193, 82, 134, 3, 72, 112, 205, 96, 138, 1, 0, 223, 114, 56, 88, 31, 253, 58, 138, 0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130 };
const unsigned char button_stop_24[] = { 137, 80, 78, 71, 13, 10, 26, 10, 0, 0, 0, 13, 73, 72, 68, 82, 0, 0, 0, 24, 0, 0, 0, 24, 8, 4, 0, 0, 0, 74, 126, 245, 115, 0, 0, 0, 51, 73, 68, 65, 84, 56, 79, 99, 96, 24, 5, 164, 130, 66, 134, 93, 12, 91, 208, 224, 46, 160, 40, 78, 112, 128, 225, 63, 22, 120, 0, 183, 134, 93, 88, 53, 236, 26, 213, 64, 103, 13, 36, 71, 28, 201, 73, 99, 20, 96, 7, 0, 200, 218, 130, 41, 127, 18, 20, 95, 0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130 };
const unsigned char button_stop_32[] = { 137, 80, 78, 71, 13, 10, 26, 10, 0, 0, 0, 13, 73, 72, 68, 82, 0, 0, 0, 32, 0, 0, 0, 32, 8, 4, 0, 0, 0, 217, 115, 178, 127, 0, 0, 0, 37, 73, 68, 65, 84, 72, 199, 99, 96, 24, 5, 163, 0, 27, 216, 192, 240, 31, 15, 36, 2, 252, 31, 53, 96, 212, 128, 81, 3, 6, 137, 1, 163, 96, 20, 144, 1, 0, 197, 28, 223, 209, 12, 95, 8, 177, 0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130 };
const unsigned char button_stop_48[] = { 137, 80, 78, 71, 13, 10, 26, 10, 0, 0, 0, 13, 73, 72, 68, 82, 0, 0, 0, 48, 0, 0, 0, 48, 8, 4, 0, 0, 0, 253, 11, 49, 12, 0, 0, 0, 72, 73, 68, 65, 84, 88, 133, 237, 205, 177, 13, 0, 32, 12, 3, 65, 15, 129, 196, 14, 97, 255, 138, 146, 193, 194, 10, 49, 18, 136, 226, 223, 189, 79, 34, 34, 250, 187, 166, 40, 175, 157, 220, 103, 121, 83, 195, 7, 194, 0, 18, 0, 0, 0, 0, 0, 0, 224, 34, 176, 12, 160, 251, 64, 215, 40, 239, 224, 158, 136, 232, 101, 27, 139, 254, 240, 31, 43, 28, 35, 89, 0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130 };
const unsigned char button_stop_64[] = { 137, 80, 78, 71, 13, 10, 26, 10, 0, 0, 0, 13, 73, 72, 68, 82, 0, 0, 0, 64, 0, 0, 0, 64, 8, 4, 0, 0, 0, 0, 96, 185, 85, 0, 0, 0, 64, 73, 68, 65, 84, 104, 222, 237, 206, 65, 21, 0, 32, 8, 68, 65, 90, 83, 129, 214, 216, 1, 61, 168, 111, 102, 3, 236, 143, 0, 0, 120, 64, 70, 15, 151, 103, 2, 122, 99, 2, 4, 8, 16, 32, 64, 128, 0, 1, 2, 4, 8, 16, 32, 224, 143, 128, 26, 223, 87, 0, 0, 220, 111, 1, 101, 124, 126, 172, 20, 227, 188, 174, 0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130 };

struct Context;

#define CMD_NAME(a) const char* name() const { return a; }
#define STR_MENU(a) wxString StrMenu(const Context *) const { return _(a); }
#define STR_DISP(a) wxString StrDisplay(const Context *) const { return _(a); }
#define STR_HELP(a) wxString StrHelp() const { return _(a); }
#define CMD_TYPE(a) int Type() const { return a; }

#define CMD_ICON(icon) wxBitmap Icon(int size, wxLayoutDirection dir = wxLayout_LeftToRight) const override { \
   wxMemoryInputStream* mem = nullptr; \
   if (size == 64) mem = new wxMemoryInputStream(icon##_64, sizeof(icon##_64));                     \
	if (size == 48) mem = new wxMemoryInputStream(icon##_48, sizeof(icon##_48));                     \
	if (size == 32) mem = new wxMemoryInputStream(icon##_32, sizeof(icon##_32));                     \
	if (size == 24) mem = new wxMemoryInputStream(icon##_24, sizeof(icon##_24));                     \
	if (size == 16) mem = new wxMemoryInputStream(icon##_16, sizeof(icon##_16));                     \
   if (mem != nullptr) {if(dir != wxLayout_RightToLeft) {return wxBitmap(wxImage(*mem)); }           \
      return wxBitmap(wxImage(*mem).Mirror());                                                       \
   } };                                                                                             \

class Cmd
{
public:
   enum CommandFlags {
      /// Default command type
      COMMAND_NORMAL = 0,

      /// Invoking this command toggles a setting of some sort. Any command
      /// of this type should have IsActive implemented to signal the
      /// current state of the thing being toggled, and invoking the command
      /// twice should be a no-op
      ///
      /// This is mutually exclusive with COMMAND_RADIO
      COMMAND_TOGGLE = 1,

      /// Invoking this command sets a setting to a specific value. Any
      /// command of this type should have IsActive implemented, and if
      /// IsActive returns true, invoking the command should have no effect
      ///
      /// This is mutually exclusive with COMMAND_TOGGLE
      COMMAND_RADIO = 2,

      /// This command has an overridden Validate method
      COMMAND_VALIDATE = 4,

      /// This command's name may change based on the state of the project
      COMMAND_DYNAMIC_NAME = 8,

      /// This command's help string may change
      COMMAND_DYNAMIC_HELP = 16,

      /// This command's icon may change based on the state of the project
      COMMAND_DYNAMIC_ICON = 32
   };

   /// Command name
   virtual const char* name() const = 0;
   /// String for menu purposes including accelerators, but not hotkeys
   virtual wxString StrMenu(const Context *) const = 0;
   /// Plain string for display purposes; should normally be the same as StrMenu
   /// but without accelerators
   virtual wxString StrDisplay(const Context *) const = 0;
   /// Short help string describing what the command does
   virtual wxString StrHelp() const = 0;

   /// Get this command's type flags
   /// @return Bitmask of CommandFlags
   virtual int Type() const { return COMMAND_NORMAL; }

   /// Request icon.
   /// @param size Icon size.
   virtual wxBitmap Icon(int size, wxLayoutDirection = wxLayout_LeftToRight) const { return wxBitmap{}; }

   /// Command function
   virtual void operator()(const Context *c) = 0;

   /// Check whether or not it makes sense to call this command at this time
   /// @param c Project context
   ///
   /// This function should be very fast, as it is called whenever a menu
   /// containing this command is opened and is called periodically for
   /// any commands used in a toolbar
   ///
   /// Note that it is still legal to call commands when this returns
   /// false. In this situation, commands should do nothing.
   ///
   /// This function should be overridden iff the command's type flags
   /// include COMMAND_VALIDATE
   virtual bool Validate(const Context *c) { return true; }

   /// Is the selectable value represented by this command currently selected?
   /// @param c Project context
   ///
   /// As with Validate, this function should be very fast.
   ///
   /// This function should be overridden iff the command's type flags
   /// include COMMAND_TOGGLE or COMMAND_RADIO
   virtual bool IsActive(const Context *c) { return false; }

   /// Destructor
   virtual ~Cmd() = default;
   virtual void SetToolID(uint32_t id) { m_id = id; };
   virtual uint32_t GetToolID() { return m_id; };
protected:
   uint32_t m_id;
};

class CmdPlay : public Cmd 
{
public:
   CMD_NAME("Play")
   CMD_ICON(button_play)
   STR_MENU("&Play")
   STR_DISP("Play")
   STR_HELP("Nacisnij aby wystartowac.")
   CMD_TYPE(COMMAND_VALIDATE)


   virtual bool Validate(const Context *c)
   {
      if (c)
         return true;
      return false;
   };

   virtual void operator()(const Context*c)
   {
      LOGINFO("Play pressed");
   };
};

class CmdStop : public Cmd
{
public:
   CMD_NAME("Stop")
   CMD_ICON(button_stop)
   STR_MENU("&Stop video")
   STR_DISP("Stop video")
   STR_HELP("Przerwij ogladanie")
   CMD_TYPE(COMMAND_VALIDATE)

   virtual bool Validate(const Context *c)
   {
      if (c != nullptr)
         return true;
      return false;
   };

   virtual void operator()(const Context*c) override {
      LOGINFO("Stop pressed");
   };
};

class CmdPause : public Cmd
{
public:
   CMD_NAME("Pause")
   CMD_ICON(button_pause)
   STR_MENU("P&ause video")
   STR_DISP("Pause video")
   STR_HELP("Zatrzymaj ogladanie")
   CMD_TYPE(COMMAND_VALIDATE)

   virtual bool Validate(const Context *c) override
   {
   if (c)
      return true;
   return false;
   };

   virtual void operator()(const Context*c) override {
      LOGINFO("Pause pressed");
   };
};

