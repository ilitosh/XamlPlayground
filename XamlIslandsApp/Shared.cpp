#include "pch.h"

#include "Shared.h"

void PopulateUI(StackPanel xamlContainer) {
  xamlContainer.HorizontalAlignment(HorizontalAlignment::Left);

  TextBox tb;
  tb.PlaceholderText(L"Placeholder");
  tb.IsSpellCheckEnabled(true);
  tb.AcceptsReturn(true);
  tb.TextWrapping(Windows::UI::Xaml::TextWrapping::Wrap);
  tb.Width(400);
  tb.Height(300);

  xamlContainer.Children().Append(tb);
}
