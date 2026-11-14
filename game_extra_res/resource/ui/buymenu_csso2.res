"Resource/UI/BuyMenu.res"
{
 "buy" { "fieldName" "buy" "PaintBackgroundType" "0" "bgcolor_override" "0 0 0 128" "infocus_bgcolor_override" "0 0 0 128" "outoffocus_bgcolor_override" "0 0 0 128" }
 "MoneyLabel" { "ControlName" "Label" "fieldName" "MoneyLabel" "font" "BuyMenuAccountFont" "fgcolor_override" "255 255 255 255" "textAlignment" "west" }
 "BuyTimeLeftLabel" { "ControlName" "Label" "fieldName" "BuyTimeLeftLabel" "font" "BuyMenuBuyTimeRemainingFont" "textAlignment" "center" "visible" "1" "fgcolor_override" "255 255 255 255" }
 "ItemNameLabel" { "ControlName" "Label" "fieldName" "ItemNameLabel" "font" "BuyMenuItemPreviewNameFont" "visible" "0" "textAlignment" "west" "fgcolor_override" "255 255 255 255" }
 "ItemDescriptionLabel" { "ControlName" "Label" "fieldName" "ItemDescriptionLabel" "font" "BuyMenuItemPreviewDescriptionFont" "wrap" "1" "visible" "0" "textAlignment" "north-west" "PaintBackgroundType" "0" "bgcolor_override" "0 0 0 51" "fgcolor_override" "255 255 255 255" }
 "SpecialMessageLabel" { "ControlName" "Label" "fieldName" "SpecialMessageLabel" "font" "BuyMenuSpecialMessageFont" "visible" "0" "textAlignment" "center" "fgcolor_override" "177 175 45 255" "bgcolor_override" "0 0 0 0" }
 "PlayerModel"
 {
  "fieldName" "PlayerModel"
  "fov" "20"
  "camera_origin" "226.06 73.87 64.82"
  "camera_angles" "2.75 -162.13 0"
  "lights"
  {
   "ambient_light" "0.23 0.27 0.36"
   "directional_light_0" { "direction" "-0.5 0.8 0" "color" "0.84 0.84 0.88" }
   "directional_light_1" { "direction" "0.7 -0.8 0" "color" "0.28 0.4 0.52" }
  }
 }
 "CancelButton"
 {
  "ControlName" "Button" "fieldName" "CancelButton" "labelText" "#CStrike_BuyMenu_Cancel" "command" "Cancel"
  "font" "BuyMenuBottomButtonFont" "textAlignment" "center" "paintborder" "0"
  "defaultBgColor_override" "0 0 0 0" "armedBgColor_override" "0 0 0 96" "textinsetx" "0" "textinsety" "0"
 }
 "BuyDropLabel"
 {
  "ControlName" "Label" "fieldName" "BuyDropLabel" "labelText" "#CStrike_BuyMenu_BuyDrop"
  "font" "BuyMenuBottomButtonFont" "textAlignment" "center" "fgcolor_override" "161 161 161 255"
 }
 "RebuyButton"
 {
  "ControlName" "Button" "fieldName" "RebuyButton" "labelText" "#CStrike_BuyMenu_ReBuy" "command" "rebuy"
  "font" "BuyMenuBottomButtonFont" "textAlignment" "center" "paintborder" "0"
  "defaultBgColor_override" "0 0 0 0" "armedBgColor_override" "0 0 0 96" "textinsetx" "0" "textinsety" "0"
 }
 "AutobuyButton"
 {
  "ControlName" "Button" "fieldName" "AutobuyButton" "labelText" "#CStrike_BuyMenu_AutoBuy" "command" "autobuy"
  "font" "BuyMenuBottomButtonFont" "textAlignment" "center" "paintborder" "0"
  "defaultBgColor_override" "0 0 0 0" "armedBgColor_override" "0 0 0 96" "textinsetx" "0" "textinsety" "0"
 }
 "SellBackAllButton"
 {
  "ControlName" "Button" "fieldName" "SellBackAllButton" "labelText" "#CStrike_BuyMenu_SellBackAll" "enabled" "0"
  "font" "BuyMenuBottomButtonFont" "textAlignment" "center" "paintborder" "0"
  "defaultBgColor_override" "0 0 0 0" "disabledBgColor_override" "0 0 0 0" "disabledFgColor_override" "133 133 133 255" "textinsetx" "0" "textinsety" "0"
 }
 "BorderBottom" { "ControlName" "Panel" "fieldName" "BorderBottom" "PaintBackgroundType" "0" "bgcolor_override" "0 0 0 128" "mouseinputenabled" "0" }
}
