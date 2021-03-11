#ifndef ESPBREW_RECIPES_H
#define ESPBREW_RECIPES_H

void MocRecipe(Schedule *nova)
{
  nova->Add(68, "Rampa 1", 60, true);
  nova->Add(78, "Mash Out", 20, true);
  nova->Add(93, "Fervura", 60);
  nova->SetUsePumpTest();
}

#endif /* ESPBREW_RECIPES_H */
