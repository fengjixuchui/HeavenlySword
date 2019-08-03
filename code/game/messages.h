//------------------------------------------------------------------------------------------
//!
//!	\file messages.h
//!
//------------------------------------------------------------------------------------------


#ifndef _MESSAGES_H
#define _MESSAGES_H


//------------------------------------------------------------------------------------------
// Forward Declares
//------------------------------------------------------------------------------------------
class Message;


//------------------------------------------------------------------------------------------
// MSG macros
//------------------------------------------------------------------------------------------
#ifndef BEGIN_MSG_LIST
	#define BEGIN_MSG_LIST enum MessageID {		\
		State_Update = -3,							\
		State_Enter = -2,							\
		State_Exit = -1,							\
		eUndefinedMsgID = 0,

	#define END_MSG_LIST };

	#define REGISTER_MSG(msg) msg,
#endif // BEGIN_MSG_LIST

BEGIN_MSG_LIST
	// IN-GAME OBJECT MESSAGES
	REGISTER_MSG(msg_detach)
	REGISTER_MSG(msg_collision)
	REGISTER_MSG(msg_obj_collision)
	REGISTER_MSG(msg_action)
	REGISTER_MSG(msg_action_specific)
	REGISTER_MSG(msg_action_specific_run)
	REGISTER_MSG(msg_action_on)
	REGISTER_MSG(msg_action_off)
	REGISTER_MSG(msg_running_action)
	REGISTER_MSG(msg_interrupt)
	REGISTER_MSG(msg_movementdone)
	REGISTER_MSG(msg_atrest)
	REGISTER_MSG(msg_deflection)
	REGISTER_MSG(msg_hitsolid)
	REGISTER_MSG(msg_hitcharacter)
	REGISTER_MSG(msg_hitragdoll)
	REGISTER_MSG(msg_animdone)
	REGISTER_MSG(msg_activate)
	REGISTER_MSG(msg_deactivate)
	REGISTER_MSG(msg_end_ragdollhold)

	// COMBAT
	REGISTER_MSG(msg_power_on)
	REGISTER_MSG(msg_power_off)
	REGISTER_MSG(msg_range_on)
	REGISTER_MSG(msg_range_off)
	REGISTER_MSG(msg_attack_on)
	REGISTER_MSG(msg_attack_off)
	REGISTER_MSG(msg_grab_on)
	REGISTER_MSG(msg_grab_off)
	REGISTER_MSG(msg_antigrav_off)
	REGISTER_MSG(msg_aim_on)
	REGISTER_MSG(msg_aim_off)
	REGISTER_MSG(msg_sword_strike)
	REGISTER_MSG(msg_ragdoll_strike)
	REGISTER_MSG(msg_dropweapons)
	REGISTER_MSG(msg_combat_safetytransitiondone_attacker)
	REGISTER_MSG(msg_combat_safetytransitiondone_receiver)
	REGISTER_MSG(msg_combat_superstyledone_attacker)
	REGISTER_MSG(msg_combat_superstyledone_receiver)
	
	// STATE MESSAGES
	REGISTER_MSG(msg_exitstate)

	// THINK OR ENTITY SPECIFIC MESSAGES
	REGISTER_MSG(msg_think_pushon)
	REGISTER_MSG(msg_think_fastpushon)
	REGISTER_MSG(msg_think_onremoveobject)
	REGISTER_MSG(msg_think_onquickthrow)
	REGISTER_MSG(msg_think_onquickthrowcheck)
	REGISTER_MSG(msg_think_onreparent)
	REGISTER_MSG(msg_think_onaftertouch)
	REGISTER_MSG(msg_think_onthrow)
	REGISTER_MSG(msg_think_ondrop)
	REGISTER_MSG(msg_think_onspawnprojectile)
	REGISTER_MSG(msg_think_ondestroyprojectile)
	REGISTER_MSG(msg_think_bazookaignition)
	REGISTER_MSG(msg_allow_reuse)
	REGISTER_MSG(msg_fire_next_shot)
	REGISTER_MSG(msg_specific_projcol)
	REGISTER_MSG(msg_barrage_setradius)

	// NEW MESSAGES - ADDED DURING LUA TO C++ CONVERSION
	REGISTER_MSG(msg_collapsed)

	// SPECIFIC MESSAGES
	REGISTER_MSG(msg_ignite)
	
	//Collapse-able.
	REGISTER_MSG(msg_destroy)
	REGISTER_MSG(msg_projcol)
	REGISTER_MSG(msg_blast_damage)
	REGISTER_MSG(msg_recoilstrike)
	REGISTER_MSG(msg_speed_attack)
	REGISTER_MSG(msg_power_attack)
	REGISTER_MSG(msg_range_attack)
	REGISTER_MSG(msg_object_strike)
	REGISTER_MSG(msg_action1)
	REGISTER_MSG(msg_action2)
	REGISTER_MSG(msg_action3)
	REGISTER_MSG(msg_action4)
	REGISTER_MSG(msg_action5)
	REGISTER_MSG(msg_smash)

	//Spear
	REGISTER_MSG(msg_removeragdoll)

	//Moving platforms.
	REGISTER_MSG(msg_moveforward)
	REGISTER_MSG(msg_movereverse)
	REGISTER_MSG(msg_movingplatform_settoend)
	REGISTER_MSG(msg_platform_notifyonmovecomplete)
	REGISTER_MSG(msg_platform_movementfinished)

	//Door
	REGISTER_MSG(Door_Open)
	REGISTER_MSG(Door_Close)
	REGISTER_MSG(Door_Slam)
	REGISTER_MSG(InUse)

	// Simple switch
	REGISTER_MSG(Deactivate)
	REGISTER_MSG(Activate)
	REGISTER_MSG(msg_think_onactivate)

	// Ballista
	REGISTER_MSG(msg_exit_ballista)
	REGISTER_MSG(msg_projectile_destroyed)

	// Player
	REGISTER_MSG( msg_forceequiprangedweapon )
	REGISTER_MSG( msg_combat_aerialended )
	REGISTER_MSG( msg_external_control_start )
	REGISTER_MSG( msg_combat_recovered )
	REGISTER_MSG( msg_combat_startstrikefailedrecovery )
	REGISTER_MSG( msg_combat_startrecovery )
	REGISTER_MSG( msg_combat_autolinked )
	REGISTER_MSG( msg_combat_attackstarted )
	REGISTER_MSG( msg_combat_impaled )
	REGISTER_MSG( msg_combat_struck )
	REGISTER_MSG( msg_combat_struck_uninterruptible )
	REGISTER_MSG( msg_buttonattack )
	REGISTER_MSG( msg_buttongrab )
	REGISTER_MSG( msg_buttonaction )
	REGISTER_MSG( msg_buttondodge )
	REGISTER_MSG( msg_combat_stanceswitch )
	REGISTER_MSG( msg_aware_gained_lockon )
	REGISTER_MSG( msg_walk_run_stopped )
	REGISTER_MSG( msg_aware_lost_lockon )
	REGISTER_MSG( msg_button_special )
	REGISTER_MSG( msg_combat_killed )
	REGISTER_MSG( msg_combat_enemy_ko )
	REGISTER_MSG( msg_combat_breakout )
	REGISTER_MSG( msg_rangefastsweep )
	REGISTER_MSG( msg_combatsyncdreactionend )
	REGISTER_MSG( msg_combat_countered )
	REGISTER_MSG( msg_combat_floored )
	REGISTER_MSG( msg_activateragdoll )
	REGISTER_MSG( msg_external_control_end )
	REGISTER_MSG( msg_button_power )
	REGISTER_MSG( msg_button_range )
	REGISTER_MSG( msg_release_attack )
	REGISTER_MSG( msg_release_grab )
	REGISTER_MSG( msg_release_action )
	REGISTER_MSG( msg_release_power )
	REGISTER_MSG( msg_release_range )
	REGISTER_MSG( msg_run_moveto_done )
	REGISTER_MSG( msg_walk_moveto_done )
	REGISTER_MSG( msg_health_changed )

	//REGISTER_MSG()
	//REGISTER_MSG()

	// Archer
	REGISTER_MSG(msg_vault_completed)
	REGISTER_MSG(msg_vault_interrupted)
	REGISTER_MSG(msg_combat_state)
	REGISTER_MSG(msg_combat_exit)
	REGISTER_MSG(msg_cut_the_stan_cam)
	REGISTER_MSG(msg_combat_attack)
	REGISTER_MSG(msg_combat_firing_xbow)
	REGISTER_MSG(msg_enter_aftertouchcheck)
	REGISTER_MSG(msg_combat_endaftertouch)
	REGISTER_MSG(msg_combat_startaftertouch)
	REGISTER_MSG(msg_combat_aftertouch_failed)
	REGISTER_MSG(msg_combat_fire)
	REGISTER_MSG(msg_combat_fire_complete)
	REGISTER_MSG(msg_combat_finished)

	//AI
	REGISTER_MSG(msg_interact)
	REGISTER_MSG(msg_interactnamed)
	REGISTER_MSG(msg_ai_attack)
	REGISTER_MSG(msg_combat_ragdollfloored)
	REGISTER_MSG(msg_combat_rise_wait)
	REGISTER_MSG(msg_throw_end)
	REGISTER_MSG(msg_dropragdoll)
	REGISTER_MSG(msg_waiting_done)
	REGISTER_MSG(msg_goto_defaultstate)
	REGISTER_MSG(msg_new_attack_target)
	REGISTER_MSG(msg_ai_hit_by_kai_nearby)

	/// Formation
	REGISTER_MSG(msg_formation_exited)

	//Ranged weaponry
	REGISTER_MSG( msg_equip )
	REGISTER_MSG( msg_start_aim )
	REGISTER_MSG( msg_aftertouch )
	REGISTER_MSG( msg_aftertouch_done )
	REGISTER_MSG( msg_control_end )
	REGISTER_MSG( msg_removefromworld )
	REGISTER_MSG( msg_projectile_aftertouch_complete )
	REGISTER_MSG( msg_ko_aftertouch )
	REGISTER_MSG( msg_weaponpickup_success )
	REGISTER_MSG( msg_ricochet )
	REGISTER_MSG( msg_item_held )
	REGISTER_MSG( msg_item_held_end )
	REGISTER_MSG( msg_combat_missedprojectilecounter )
	REGISTER_MSG( msg_projectile_hitranged )			//When the heroine hits the projectiles with ranged-attack.
	

	//Turret point
	REGISTER_MSG( msg_ninja_sequence_success )
	REGISTER_MSG( msg_ninja_sequence_fail )
	REGISTER_MSG( msg_ninja_sequence_die )
	REGISTER_MSG( msg_turret_point )
	REGISTER_MSG( msg_turret_cablecar )
	REGISTER_MSG( msg_return_state )

	REGISTER_MSG( msg_success )
	REGISTER_MSG( msg_fail )

	// Kite
	REGISTER_MSG( msg_attach )
	REGISTER_MSG( Trigger )
	REGISTER_MSG( msg_landed )

	// Catapult
	REGISTER_MSG( msg_move )
	REGISTER_MSG( msg_stop )
	REGISTER_MSG( msg_target )
	REGISTER_MSG( msg_fire )
	REGISTER_MSG( msg_fire_volley )
	REGISTER_MSG( msg_reset )
	REGISTER_MSG( msg_reload )
	REGISTER_MSG( msg_fire_complete )
	REGISTER_MSG( msg_reload_complete )
	REGISTER_MSG( msg_activate_usepoint )
	REGISTER_MSG( msg_deactivate_usepoint )
	REGISTER_MSG( msg_explode_catapult )

	// Interactable_Thrown
	REGISTER_MSG( msg_goto_attachedstate ) //Set it from default state into attached state, use msg_detach_velocity to unattach them whenever.
	REGISTER_MSG( msg_detach_velocity )

	// Bosses
	REGISTER_MSG( msg_ggen_shellbreak_01 )
	REGISTER_MSG( msg_ggen_shellbreak_02 )
	REGISTER_MSG( msg_ggen_shellbreak_03 )
	REGISTER_MSG( msg_ggen_stop_pillar_notification )
	REGISTER_MSG( msg_ggen_stomplanded )
	REGISTER_MSG( msg_agen_unholster_sword_left )
	REGISTER_MSG( msg_agen_unholster_sword_right )
	REGISTER_MSG( msg_agen_holster_sword_left )
	REGISTER_MSG( msg_agen_holster_sword_right )
	REGISTER_MSG( msg_agen_unholster_powersword_left )
	REGISTER_MSG( msg_agen_unholster_powersword_right )
	REGISTER_MSG( msg_agen_holster_powersword_left )
	REGISTER_MSG( msg_agen_holster_powersword_right )
	REGISTER_MSG( msg_demon_playanim )
	REGISTER_MSG( msg_startbossbattle )
	REGISTER_MSG( msg_bosscombat_struck_invulnerable )
	REGISTER_MSG( msg_boss_gotophase )
	REGISTER_MSG( msg_headshot )
	REGISTER_MSG( msg_aerialgeneralhack_forceherotointeracting )

	// Animated
	REGISTER_MSG(PlayAnim)
	REGISTER_MSG(msg_anim_play)
	REGISTER_MSG(msg_anim_stop)
	REGISTER_MSG(msg_anim_speed)
	REGISTER_MSG(msg_anim_looping)
	REGISTER_MSG(msg_animated_settoend)

	// Spawn Points
	REGISTER_MSG(msg_spawn)
	REGISTER_MSG(msg_spawned)
	REGISTER_MSG(msg_spawn_fail)
	REGISTER_MSG(msg_spawn_succeed)

	// Objective
	REGISTER_MSG(msg_time_complete)
	REGISTER_MSG(msg_bodycount_complete)

	REGISTER_MSG(msg_army_start)
	REGISTER_MSG(msg_army_end)
	REGISTER_MSG(msg_army_fail)
	REGISTER_MSG(msg_army_global_event)

	//Sprite
	REGISTER_MSG(msg_sprite_queueanim)
	REGISTER_MSG(msg_sprite_notifyoncomplete)

	/*
	REGISTER_MSG(  )
	REGISTER_MSG(  )
	REGISTER_MSG(  )
	REGISTER_MSG(  )
	REGISTER_MSG(  )
	REGISTER_MSG(  )
	*/

	REGISTER_MSG(msg_special_onscreen_message)

	// Movies.
	REGISTER_MSG( msg_movie_finished )

END_MSG_LIST

#endif //_MESSAGES_H
