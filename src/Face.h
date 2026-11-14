#pragma once

// Your character's face during conversations (0.8, first step). The game keeps your face
// neutral and your lines carry no expression tags, so the face is set here: while the NPC
// speaks, a response to their line's expression; when you choose an option with a skill
// tag ([Persuade], [Intimidate]...), an expression for it. Expressions are the game's
// facial archetypes (AnimFaceArchetype* keywords), set with the function behind Papyrus'
// Actor.ChangeAnimFaceArchetype (found by rrrafau for PointCameraAtPlayer). Main thread.
namespace CCC::Face
{
	// The expression your face takes while listening to a line with this tag ("" = none).
	[[nodiscard]] std::string ListeningTo(std::string_view a_npcTag);

	// The expression for a dialogue option you chose, from its skill tag ("" = none).
	[[nodiscard]] std::string ForChoice(std::string_view a_optionText);

	// Sets your expression ("" puts the game's own back). a_why is for the log.
	void Set(const std::string& a_expression, std::string_view a_why);

	// Your background's signature expression ("" if none or unknown), a_background its name.
	// Read once per conversation.
	[[nodiscard]] std::string Signature(std::string* a_background = nullptr);

	// With iSignatureChance percent: your signature expression, else "". For moments the
	// face would otherwise go back to neutral.
	[[nodiscard]] std::string MaybeSignature();

	// What the last MaybeSignature came from (a background's name, or "Wanted").
	[[nodiscard]] const std::string& SignatureSource();

	// Your emotion close-up chance with your traits applied (Introvert half, Extrovert half
	// again). Any thread.
	[[nodiscard]] int CloseUpChance(int a_percent);

	// The expression set now ("" = the game's own).
	[[nodiscard]] const std::string& Current();

	// Main thread, every tick: puts your face back once the conversation isn't filmed.
	void Update(bool a_filming);
}
