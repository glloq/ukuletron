# Étude : Contrôle PWM des Solénoïdes avec PCA9685

## 1. CONTEXTE DU PROJET

### 1.1 Configuration actuelle
- **32 solénoïdes** (9+9+8+6 par corde)
- **Contrôle digital ON/OFF** via 2x MCP23017
- **Temps d'activation maximum : 5000ms** (5 secondes)
- **Délai avant pluck : 20ms** (ligne 49, ukuleleString.cpp)
- **Problème identifié** : Surchauffe des solénoïdes avec contrôle digital pur

### 1.2 Objectif de l'étude
Analyser la faisabilité et les paramètres d'utilisation du PCA9685 pour :
- Réduire la consommation électrique
- Limiter la surchauffe des solénoïdes
- Maintenir une force de frappe suffisante
- Implémenter un "holding" PWM après activation

---

## 2. ANALYSE DE PUISSANCE

### 2.1 Spécifications typiques des solénoïdes 7N

Pour un solénoïde push-pull de 7N (force recommandée dans le README) :

| Paramètre | Valeur typique | Notes |
|-----------|----------------|-------|
| Tension nominale | 5V DC | Alimentation actuelle du système |
| Courant nominal | 400-600 mA | À vérifier selon modèle exact |
| Résistance bobine | 8-12 Ω | Typique pour 5V |
| Force de frappe | 7N (≈700g) | Suffisant pour la plupart des frettes |
| Course utile | 3-5 mm | Pour appuyer sur les cordes |

### 2.2 Calcul de puissance

#### Mode digital actuel (100% duty cycle) :
```
Puissance par solénoïde = V² / R = 5² / 10 = 2.5W
Courant = V / R = 5 / 10 = 0.5A
```

#### Consommation maximale théorique :
```
32 solénoïdes × 2.5W = 80W (si tous actifs simultanément)
32 solénoïdes × 0.5A = 16A (courant total max)
```

#### En pratique (usage réel) :
- **Polyphonie moyenne** : 4-6 notes simultanées
- **Consommation réelle** : 10-15W en usage normal
- **Pics de courant** : 2-3A lors d'accords complexes

### 2.3 Problématique thermique

**Énergie dissipée en chaleur** (mode digital continu) :
```
Pour 1 note tenue 5 secondes :
Énergie = P × t = 2.5W × 5s = 12.5 Joules

Pour 1 note tenue 1 minute (usage excessif) :
Énergie = 2.5W × 60s = 150 Joules
```

**Conséquences** :
- Échauffement rapide de la bobine (> 60-80°C possible)
- Dégradation de l'isolation du fil émaillé
- Perte de force par augmentation de résistance
- Risque de désaimantation permanente

---

## 3. TEMPS D'ACTIVATION ET CYCLES THERMIQUES

### 3.1 Analyse du code actuel

#### Paramètres de timing (settings.h:65) :
```cpp
constexpr int MAX_ACTIVATION_TIME = 5000; // 5 secondes max
```

#### Cycle d'activation (ukuleleString.cpp:86-92) :
```
1. Note ON → digitalWrite(HIGH)
2. Delay 20ms
3. Pluck servo
4. Maintien jusqu'à Note OFF ou timeout 5s
5. Note OFF → digitalWrite(LOW)
```

### 3.2 Scénarios d'utilisation

| Scénario | Durée activation | Fréquence | Risque thermique |
|----------|------------------|-----------|------------------|
| Staccato | < 100ms | Élevée | **FAIBLE** - Refroidissement entre notes |
| Notes courtes | 100-500ms | Moyenne | **FAIBLE** - Dissipation suffisante |
| Notes tenues | 0.5-2s | Variable | **MOYEN** - Commence à chauffer |
| Accords longs | 2-5s | Faible | **ÉLEVÉ** - Accumulation thermique |
| Boucle continue | 5s répétés | Élevée | **CRITIQUE** - Surchauffe garantie |

### 3.3 Temps d'activation critique

**Sans PWM (digital pur)** :
- ⚠️ **> 3 secondes** : Échauffement notable (40-50°C)
- 🔥 **> 5 secondes** : Surchauffe (60-80°C)
- 💀 **> 10 secondes** : Dommages potentiels

**Avec PWM holding** (scénario souhaité) :
- ✅ **0-50ms** : 100% duty cycle (frappe initiale)
- ✅ **50ms-∞** : 30-50% duty cycle (maintien)
- 🎯 **Durée illimitée possible** avec bon dimensionnement

---

## 4. STRATÉGIE PWM AVEC PCA9685

### 4.1 Principe du "PWM Holding"

Le contrôle PWM en deux phases permet de :
1. **Phase d'attaque (0-50ms)** : PWM à 100% pour frappe rapide et forte
2. **Phase de maintien (50ms+)** : PWM réduit (30-50%) pour tenir la corde

**Justification physique** :
- La force nécessaire pour **déplacer** le piston (inertie) > force pour **maintenir** la position
- Réduction de courant de 50% → Réduction de puissance de 75% (P = I²R)
- Température d'équilibre thermique divisée par ~4

### 4.2 Spécifications du PCA9685

| Caractéristique | Valeur | Application solénoïdes |
|-----------------|--------|------------------------|
| Canaux PWM | 16 | 16 solénoïdes par PCA9685 → **2 modules requis** |
| Fréquence PWM | 40-1600 Hz | Recommandé : **500-1000 Hz** |
| Résolution | 12 bits (4096 niveaux) | Précision : 0.024% par step |
| Interface | I2C | Adresses : 0x40-0x7F (jusqu'à 62 modules) |
| Tension logique | 3-5V | Compatible Arduino 5V |
| Courant sortie | 25mA max | ⚠️ **Nécessite drivers MOSFET** |

### 4.3 Architecture de contrôle

```
┌─────────────┐      I2C       ┌──────────┐      PWM      ┌─────────┐
│  Arduino    │─────────────────│ PCA9685  │───────────────│ MOSFET  │
│  Leonardo   │                 │  #1      │               │ Driver  │
│             │                 │ (0x40)   │───┐           │ IRF540N │
└─────────────┘                 └──────────┘   │           └─────────┘
                                                │                │
                                ┌──────────┐   │           ┌────▼─────┐
                                │ PCA9685  │   │           │ Solénoïde│
                                │  #2      │───┘           │  + Diode │
                                │ (0x41)   │               │  Roue    │
                                └──────────┘               │  Libre   │
                                                           └──────────┘
```

**Composants requis par solénoïde** :
- 1× MOSFET N-Channel (IRF540N, IRLZ44N, etc.) - VDS > 20V, ID > 1A
- 1× Diode de roue libre (1N4007 ou Schottky 1A)
- 1× Résistance gate 1kΩ (protection)

### 4.4 Calcul des paramètres PWM

#### Fréquence PWM optimale :

| Fréquence | Avantages | Inconvénients | Recommandation |
|-----------|-----------|---------------|----------------|
| 100-200 Hz | Force maximale, vibration audible | Bruit mécanique | ❌ Trop bruyant |
| 500-800 Hz | Bon compromis silence/force | Légère perte de force | ✅ **OPTIMAL** |
| 1000-1600 Hz | Silencieux | Perte de force, échauffement driver | ⚠️ Acceptable |

**Recommandation : 500 Hz** (période = 2ms)

#### Duty cycle pour maintien :

Tests empiriques recommandés, estimation initiale :

```
Phase d'attaque (0-50ms) :
- Duty cycle : 100% (valeur PCA9685 : 4095)
- Courant : 500mA
- Puissance : 2.5W

Phase de maintien (50ms+) :
- Duty cycle : 40% (valeur PCA9685 : 1638)
- Courant : 200mA (estimation)
- Puissance : 1.0W (réduction de 60%)
```

**Température d'équilibre estimée** :
```
Sans PWM : 80°C (après 30s continu)
Avec PWM 40% : 35-40°C (acceptable pour usage continu)
```

---

## 5. CALIBRATION ET TESTS REQUIS

### 5.1 Protocole de test de force

**Objectif** : Déterminer le duty cycle minimum pour maintien

1. **Setup** :
   - Monter 1 solénoïde sur le manche
   - Appliquer PWM à différents duty cycles
   - Mesurer la force résiduelle avec un dynamomètre

2. **Procédure** :
   ```
   Pour duty_cycle de 20% à 100% (step 5%) :
     - Activer solénoïde avec duty_cycle
     - Attendre stabilisation (500ms)
     - Mesurer force de maintien
     - Tester si la corde est bien appuyée (son propre)
   ```

3. **Critères de validation** :
   - Force minimum : 3-4N (suffisant pour maintien)
   - Son clair sans buzz
   - Pas de vibration audible

4. **Résultat attendu** :
   - Duty cycle optimal : probablement entre 30-50%
   - Variation selon position sur manche (première frette > dernière frette)

### 5.2 Protocole de test thermique

**Objectif** : Vérifier que la température reste acceptable

1. **Setup** :
   - Thermocouple ou thermomètre IR
   - Activation continue pendant 5 minutes
   - Mesure toutes les 30 secondes

2. **Conditions de test** :
   ```
   Test A : Digital pur (100%)
   Test B : PWM 50%
   Test C : PWM 40%
   Test D : PWM 30%
   ```

3. **Critères de réussite** :
   - Température max < 50°C (sécuritaire long terme)
   - Temps pour atteindre équilibre < 2 minutes
   - Température ambiante après 5min arrêt < 30°C

### 5.3 Protocole de test de timing

**Objectif** : Optimiser le délai de transition attaque → maintien

1. **Test du délai avant réduction** :
   ```
   Pour delay de 10ms à 200ms (step 10ms) :
     - Phase attaque 100% pendant 'delay'
     - Transition vers 40%
     - Écouter le son produit
     - Noter si attaque suffisante
   ```

2. **Résultat attendu** :
   - Délai minimal : probablement 30-50ms
   - Doit permettre course complète du piston

---

## 6. COMPARAISON MCP23017 vs PCA9685

### 6.1 Tableau comparatif

| Critère | MCP23017 (actuel) | PCA9685 (proposé) |
|---------|-------------------|-------------------|
| **Fonction** | GPIO digital I/O | PWM 12-bit |
| **Canaux** | 16 GPIO | 16 PWM |
| **Résolution** | ON/OFF (1 bit) | 4096 niveaux (12 bits) |
| **Fréquence** | N/A (statique) | 40-1600 Hz configurable |
| **Interface** | I2C | I2C |
| **Adresses I2C** | 0x20-0x27 (8 max) | 0x40-0x7F (62 max) |
| **Courant sortie** | 25mA max | 25mA max |
| **Drivers requis** | MOSFET (ON/OFF) | MOSFET (PWM) |
| **Contrôle puissance** | ❌ Aucun | ✅ PWM variable |
| **Gestion thermique** | ❌ Timeout logiciel uniquement | ✅ Réduction PWM matérielle |
| **Complexité code** | Simple (digitalWrite) | Moyenne (librairie Adafruit) |
| **Coût** | ~2€/module | ~3€/module |

### 6.2 Avantages du PCA9685

✅ **Réduction thermique** : Division par 2-4 de la puissance dissipée
✅ **Durée de vie** : Moins de stress thermique sur les solénoïdes
✅ **Flexibilité** : Ajustement fin de la force par logiciel
✅ **Holding efficace** : Maintien à faible puissance
✅ **Scalabilité** : Jusqu'à 62 modules sur un bus I2C
✅ **Force modulable** : Adaptation selon position frette (plus de force sur frette 1)

### 6.3 Inconvénients du PCA9685

❌ **Complexité** : Code plus complexe (gestion phases attaque/maintien)
❌ **Calibration** : Nécessite tests empiriques pour chaque configuration
❌ **Latence I2C** : Communication I2C (~100µs par commande) vs GPIO direct
❌ **Debugging** : Plus difficile à déboguer (valeurs PWM vs simple HIGH/LOW)
❌ **Compatibilité** : Nécessite réécriture de `ukuleleString.cpp`

---

## 7. IMPACT SUR L'ARCHITECTURE LOGICIELLE

### 7.1 Modifications requises

#### Fichier : `settings.h`
```
Ajouts :
- Adresses I2C des PCA9685 (0x40, 0x41)
- Constantes PWM :
  - ATTACK_DUTY_CYCLE (4095 = 100%)
  - HOLD_DUTY_CYCLE (1600-2000 = 40-50%)
  - ATTACK_DURATION_MS (30-50ms)
  - PWM_FREQUENCY (500 Hz)
```

#### Fichier : `ukuleleString.cpp`
```
Modifications de activateFret() :
1. Remplacer digitalWrite(HIGH) par :
   - setPWM(pin, 0, ATTACK_DUTY_CYCLE)
   - Timer pour transition après ATTACK_DURATION_MS

2. Ajouter méthode transitionToHold() :
   - Appelée par update() après délai
   - setPWM(pin, 0, HOLD_DUTY_CYCLE)

3. Modifier desactivateFret() :
   - setPWM(pin, 0, 0) au lieu de digitalWrite(LOW)
```

#### Nouveau fichier : `pcaDevices.h/cpp`
```
Gestion des modules PCA9685 :
- Initialisation avec adresses I2C
- Wrapper pour setPWM()
- Gestion de la fréquence globale
- Interface similaire à mcpDevices.h
```

### 7.2 Diagramme de séquence PWM

```
Note ON
   │
   ├─> activateFret()
   │     │
   │     ├─> setPWM(100%)  ─────────────┐
   │     │                               │ Phase d'attaque
   │     └─> enregistrer temps attaque  ─┘ (30-50ms)
   │
   ├─> delay(20ms)
   │
   ├─> pluck()
   │
   │
  update() (appelé en boucle)
   │
   ├─> vérifier si (millis - temps_attaque > ATTACK_DURATION)
   │     │
   │     └─> OUI: setPWM(40%)  ─────────┐
   │                                     │ Phase de maintien
   │                                     │ (jusqu'à Note OFF
   │                                     │  ou timeout 5s)
   │
Note OFF
   │
   └─> desactivateFret()
         │
         └─> setPWM(0%)
```

---

## 8. ESTIMATION DE PERFORMANCE

### 8.1 Réduction de consommation

**Scénario : Accord tenu 2 secondes (4 notes simultanées)**

| Phase | Sans PWM | Avec PWM | Réduction |
|-------|----------|----------|-----------|
| Attaque (0-50ms) | 10W | 10W | 0% |
| Maintien (50-2000ms) | 10W | 4W | **60%** |
| Moyenne pondérée | 10W | 4.15W | **58.5%** |
| Énergie totale | 20J | 8.3J | **58.5%** |

**Sur une utilisation de 10 minutes** :
- Sans PWM : ~360J (si notes courtes) à 6000J (si notes longues)
- Avec PWM : ~360J à 2500J
- **Économie : jusqu'à 3500J (≈58% sur notes longues)**

### 8.2 Réduction thermique

**Modèle simplifié** :
```
Température équilibre = T_ambiante + (Puissance × Résistance_thermique)

Sans PWM (2.5W continu) :
T_eq = 25°C + (2.5W × 22°C/W) = 80°C

Avec PWM 40% (1.0W continu) :
T_eq = 25°C + (1.0W × 22°C/W) = 47°C
```

**Résultat** : Température réduite de **80°C à 47°C** (-41%)

### 8.3 Impact sur alimentation

**Alimentation actuelle** : 5V 2A (10W max)

**Consommation système** :
- Arduino Leonardo : ~100mA (0.5W)
- 4× Servos au repos : ~200mA (1W)
- 2× MCP23017/PCA9685 : ~20mA (0.1W)

**Budget solénoïdes** :
```
Sans PWM :
- Max théorique : 10W - 1.6W = 8.4W → ~3 solénoïdes max

Avec PWM (maintien 40%) :
- Phase attaque : 10W - 1.6W = 8.4W → 3 solénoïdes
- Phase maintien : 8.4W / 1W = 8 solénoïdes simultanés OK
```

**Conclusion** :
- ⚠️ **Sans PWM** : Alimentation sous-dimensionnée (risque de brown-out)
- ✅ **Avec PWM** : Alimentation suffisante pour usage normal

---

## 9. RECOMMANDATIONS

### 9.1 Stratégie d'implémentation

**Phase 1 : Prototypage (1 solénoïde)**
1. ✅ Commander 1× PCA9685 + drivers MOSFET
2. ✅ Tester sur 1 solénoïde monté
3. ✅ Calibrer duty cycle optimal (tests force/thermique)
4. ✅ Mesurer température en usage réel
5. ✅ Valider son produit et réactivité

**Phase 2 : Intégration partielle (1 corde)**
1. ✅ Implémenter code PWM pour 1 corde (9 solénoïdes)
2. ✅ Tester en conditions réelles (MIDI)
3. ✅ Ajuster timings attaque/maintien
4. ✅ Valider stabilité I2C et latences

**Phase 3 : Déploiement complet**
1. ✅ Commander 2× PCA9685 (pour 32 solénoïdes)
2. ✅ Migrer toutes les cordes
3. ✅ Retirer les MCP23017 (ou les garder pour GPIO futures)
4. ✅ Tests thermiques longue durée
5. ✅ Optimisation finale des paramètres

### 9.2 Paramètres initiaux recommandés

```cpp
// À ajouter dans settings.h

// Adresses PCA9685
#define PCA9685_ADDR_1  0x40
#define PCA9685_ADDR_2  0x41

// Fréquence PWM
#define PWM_FREQUENCY   500  // Hz

// Duty cycles (sur 4096)
#define ATTACK_DUTY     4095  // 100%
#define HOLD_DUTY       1638  // 40%

// Timings
#define ATTACK_PHASE_MS 40    // Durée phase attaque
```

**À ajuster après tests empiriques** :
- `HOLD_DUTY` : 30-50% selon force nécessaire
- `ATTACK_PHASE_MS` : 30-60ms selon réactivité solénoïde

### 9.3 Matériel recommandé

**PCA9685** :
- Adafruit PCA9685 16-Channel PWM Driver (ID: 815) - 15€
- Générique compatible (attention qualité I2C) - 3-5€

**MOSFETs** :
- IRLZ44N (Logic Level, VDS=55V, ID=47A) - Recommandé
- IRF540N (nécessite pull-up 10kΩ pour 5V)
- AO3400 (SOT-23, pour PCB compact)

**Diodes de roue libre** :
- 1N4007 (1A, 1000V) - Pas cher, robuste
- 1N5819 (Schottky, récupération rapide) - Préférable

**Connectique** :
- Borniers à vis 5.08mm pour solénoïdes
- Câbles dupont femelle-femelle pour PCA9685

### 9.4 Alimentation recommandée

**Mise à niveau conseillée** :
```
Actuelle : 5V 2A (10W) → ⚠️ Insuffisante
Recommandée : 5V 4A (20W) → ✅ Confortable

Justification :
- Pics d'attaque : 3-4 solénoïdes × 2.5W = 7.5-10W
- Maintien : 6-8 solénoïdes × 1W = 6-8W
- Arduino + servos : 1.5W
- Total avec marge : 15-20W
```

**Alimentation suggérée** :
- Mean Well RS-25-5 (5V 5A, 25W) - Fiable, industrielle
- Alimentation PC 5V rail (récupération)

---

## 10. RISQUES ET LIMITATIONS

### 10.1 Risques techniques

| Risque | Probabilité | Impact | Mitigation |
|--------|-------------|--------|------------|
| Latence I2C excessive | Moyenne | Moyen | Tests temps réel, optimiser fréquence I2C à 400kHz |
| Calibration complexe | Élevée | Faible | Documenter procédure, valeurs par défaut conservatrices |
| Bruit audible PWM | Moyenne | Moyen | Fréquence > 500Hz, tests acoustiques |
| Interférence I2C | Faible | Élevé | Câbles courts, condensateurs découplage |
| Surchauffe drivers | Faible | Moyen | Dimensionner MOSFETs, dissipateurs si nécessaire |

### 10.2 Limitations du PCA9685

❌ **Pas de feedback** : Impossible de détecter un solénoïde défaillant
❌ **Résolution temporelle** : Transition attaque→maintien = ±1-2ms de jitter
❌ **Fréquence fixe** : Tous les canaux à la même fréquence PWM
❌ **Courant limité** : 25mA par sortie → drivers obligatoires

### 10.3 Plan B (si PWM échoue)

Si les tests montrent que le PWM n'améliore pas suffisamment :

**Alternative 1** : Solénoïdes basse puissance
- Remplacer par modèles 3.3V 200mA
- Alimentation dédiée 3.3V
- Moins de force mais moins de chaleur

**Alternative 2** : Limitation logicielle stricte
- Réduire MAX_ACTIVATION_TIME à 2000ms
- Ajouter temps de repos forcé entre activations
- Monitoring thermique logiciel

**Alternative 3** : Dissipation passive
- Support aluminium pour tous les solénoïdes
- Pâte thermique entre solénoïde et support
- Ventilation passive par convection

---

## 11. CONCLUSION

### 11.1 Synthèse

L'utilisation du PCA9685 pour le contrôle PWM des solénoïdes présente des **avantages significatifs** :

✅ **Réduction thermique** : -40°C en température d'équilibre
✅ **Économie d'énergie** : -58% sur notes longues
✅ **Durabilité** : Moins de stress sur les composants
✅ **Flexibilité** : Ajustement force par logiciel

**Mais nécessite** :
⚠️ Calibration empirique soigneuse
⚠️ Modifications logicielles non triviales
⚠️ Validation en conditions réelles

### 11.2 Décision recommandée

🎯 **OUI, implémenter le PWM avec PCA9685**

**Justification** :
1. Le problème thermique est réel (mentionné dans README)
2. L'architecture I2C est déjà en place
3. Le gain énergétique permet de garder l'alimentation actuelle
4. La migration peut être progressive (1 corde à la fois)
5. Coût modéré (~20€ pour 2 modules + composants)

### 11.3 Prochaines étapes

**Étape 1** : Commande matériel
- [ ] 1× PCA9685 module (tests)
- [ ] 5× IRLZ44N MOSFET
- [ ] 5× 1N5819 diode Schottky
- [ ] Breadboard pour prototypage

**Étape 2** : Tests empiriques (1 solénoïde)
- [ ] Monter circuit sur breadboard
- [ ] Tester duty cycles 20-100%
- [ ] Mesurer température vs duty cycle
- [ ] Valider son produit

**Étape 3** : Développement logiciel
- [ ] Créer branche git `feature/pwm-pca9685`
- [ ] Implémenter classe `pcaDevices`
- [ ] Modifier `ukuleleString.cpp`
- [ ] Tester sur 1 corde (9 solénoïdes)

**Étape 4** : Validation complète
- [ ] Tests MIDI en conditions réelles
- [ ] Mesures thermiques longue durée
- [ ] Ajustement paramètres finaux
- [ ] Déploiement sur les 4 cordes

---

## 12. ANNEXES

### 12.1 Bibliographie technique

**Datasheets** :
- PCA9685 : https://www.nxp.com/docs/en/data-sheet/PCA9685.pdf
- IRLZ44N : https://www.infineon.com/dgdl/irlz44npbf.pdf
- MCP23017 : https://ww1.microchip.com/downloads/en/devicedoc/20001952c.pdf

**Librairies Arduino** :
- Adafruit PWM Servo Driver Library : https://github.com/adafruit/Adafruit-PWM-Servo-Driver-Library

**Ressources solénoïdes** :
- Solenoid Basics : https://www.rs-online.com/designspark/solenoids
- PWM Control of Solenoids : https://www.electronicdesign.com/power-management/article/21799111/pwm-control-of-solenoid-valves

### 12.2 Calculs thermiques détaillés

**Modèle de transfert thermique** :
```
Résistance thermique solénoïde → air :
R_th = 20-25 °C/W (sans dissipateur)
R_th = 8-12 °C/W (avec support alu)

Constante de temps thermique :
τ = R_th × C_th ≈ 60-120 secondes

Température d'équilibre :
T_eq = T_amb + P × R_th

Exemples :
- 2.5W sans dissipateur : 25 + 2.5×22 = 80°C
- 1.0W sans dissipateur : 25 + 1.0×22 = 47°C
- 1.0W avec alu : 25 + 1.0×10 = 35°C ✅
```

### 12.3 Budget I2C

**Timing I2C (Fast Mode 400kHz)** :
```
1 transaction I2C (write) : ~50-100µs
Commande setPWM() : 1 transaction = 100µs

Pour 32 solénoïdes updated simultanément :
32 × 100µs = 3.2ms

Latence acceptable : < 5ms
Conclusion : ✅ Compatible temps réel
```

**Optimisations possibles** :
- Batch updates via PCA9685 ALL_LED registers
- Update uniquement les canaux modifiés
- Utiliser Wire.setClock(400000) pour I2C rapide

---

**Document créé le** : 2026-01-03
**Auteur** : Étude technique pour projet Ukuletron
**Version** : 1.0
**Statut** : Recommandation d'implémentation
